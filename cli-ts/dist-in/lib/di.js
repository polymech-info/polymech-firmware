import fs from 'fs/promises';
import { log } from "./logger.js";
import path from "path";
import Parser from 'tree-sitter';
import Cpp from 'tree-sitter-cpp';
const parser = new Parser();
parser.setLanguage(Cpp);
// 1. Parse a C++ file and extract all defines (commented and uncommented)
function parseDefines(fileContent) {
    const defines = new Map();
    const tree = parser.parse(fileContent);
    function traverse(node) {
        if (node.type === 'preproc_def') {
            const nameNode = node.childForFieldName('name');
            const valueNode = node.childForFieldName('value');
            if (nameNode) {
                defines.set(nameNode.text, {
                    value: valueNode?.text.trim() ?? null,
                    isComment: false
                });
            }
        }
        else if (node.type === 'comment' && node.text.includes('#define')) {
            const match = node.text.match(/#define\s+(\S+)\s*(.*)/);
            if (match) {
                defines.set(match[1], {
                    value: match[2].trim() || null,
                    isComment: true
                });
            }
        }
        else {
            for (const child of node.children) {
                traverse(child);
            }
        }
    }
    traverse(tree.rootNode);
    return defines;
}
// 2. Build a map of all possible features from features.h (Feature -> Header)
export async function buildFeatureCatalog(projectRoot) {
    const catalog = new Map();
    const featuresPath = path.join(projectRoot, 'src', 'features.h');
    log.silly(`Building feature catalog from: ${featuresPath}`);
    const featuresContent = await fs.readFile(featuresPath, 'utf-8');
    const tree = parser.parse(featuresContent);
    function findIfdefs(node) {
        if (node.type === 'preproc_ifdef') {
            const featureName = node.childForFieldName('name')?.text;
            const includePath = node.descendantsOfType('preproc_include')[0]?.childForFieldName('path')?.text.replace(/["<>]/g, '');
            if (featureName && includePath) {
                catalog.set(featureName, includePath);
            }
        }
        for (const child of node.children) {
            findIfdefs(child);
        }
    }
    findIfdefs(tree.rootNode);
    log.silly(`Found ${catalog.size} potential features in catalog.`);
    return catalog;
}
// 3. Create a map of ClassName -> FeatureFlag for all components
export async function buildClassNameToFeatureMap(featureCatalog, includeDirs) {
    const map = new Map();
    // --- AST DUMP LOGIC ---
    const astDir = path.join(process.cwd(), 'ast');
    await fs.mkdir(astDir, { recursive: true });
    function serializeNode(node) {
        const children = node.children.map(serializeNode);
        const result = { type: node.type };
        if (children.length > 0) {
            result.children = children;
        }
        else {
            result.text = node.text;
        }
        return result;
    }
    // --- END AST DUMP LOGIC ---
    for (const [featureFlag, headerFile] of featureCatalog.entries()) {
        let absolutePath = null;
        for (const dir of includeDirs) {
            const testPath = path.join(dir, headerFile);
            try {
                await fs.stat(testPath);
                absolutePath = testPath;
                break;
            }
            catch (e) { /* continue */ }
        }
        if (absolutePath) {
            const content = await fs.readFile(absolutePath, 'utf-8');
            const tree = parser.parse(content);
            const className = findClassName(tree.rootNode);
            if (className) {
                map.set(className, featureFlag);
                // --- AST DUMP FOR THIS COMPONENT ---
                const astJson = JSON.stringify(serializeNode(tree.rootNode), null, 2);
                const componentName = path.basename(headerFile, '.h');
                await fs.writeFile(path.join(astDir, `${componentName}.json`), astJson);
                log.info(`[DEBUG] AST for ${componentName} written to ${astDir}`);
                // --- END AST DUMP ---
            }
        }
    }
    log.silly(`Mapped ${map.size} class names to feature flags by parsing headers.`);
    return map;
}
// 4. For a given component header, find its dependencies
export async function analyzeComponentDependencies(headerPath, includeDirs, classNameToFeatureMap) {
    const dependencies = new Set();
    let absolutePath = null;
    for (const dir of includeDirs) {
        const testPath = path.join(dir, headerPath);
        try {
            await fs.stat(testPath);
            absolutePath = testPath;
            break;
        }
        catch (e) { /* continue */ }
    }
    if (!absolutePath) {
        log.warn(`Could not find header file: ${headerPath}`);
        return [];
    }
    log.silly(`Analyzing dependencies for: ${absolutePath}`);
    const content = await fs.readFile(absolutePath, 'utf-8');
    const tree = parser.parse(content);
    // Find dependencies from constructor
    const className = findClassName(tree.rootNode);
    if (!className)
        return [];
    const constructorDeclarations = tree.rootNode.descendantsOfType('constructor_or_destructor_declaration');
    for (const declaration of constructorDeclarations) {
        const functionName = declaration.childForFieldName('name')?.text;
        if (functionName !== className)
            continue; // It's a destructor, skip it
        const declarators = declaration.descendantsOfType('function_declarator');
        for (const declarator of declarators) {
            const params = declarator.descendantsOfType('parameter_declaration');
            for (const param of params) {
                const typeNode = param.descendantsOfType('type_identifier')[0];
                const typeName = typeNode?.text;
                if (typeName && classNameToFeatureMap.has(typeName)) {
                    dependencies.add(classNameToFeatureMap.get(typeName));
                }
            }
        }
    }
    return Array.from(dependencies);
}
function findClassName(rootNode) {
    const classSpecifier = rootNode.descendantsOfType('class_specifier')[0];
    return classSpecifier?.childForFieldName('name')?.text ?? null;
}
// Main orchestration function
export async function generateFeatureTree(projectRoot) {
    // Load and parse config.h
    const configPath = path.join(projectRoot, 'src', 'config.h');
    log.silly(`Loading config from: ${configPath}`);
    const configContent = await fs.readFile(configPath, 'utf-8');
    const allDefines = parseDefines(configContent);
    // Filter for active features
    const enabledFeatures = [];
    for (const [name, { isComment }] of allDefines.entries()) {
        if (name.startsWith('ENABLE_') && !isComment) {
            enabledFeatures.push(name);
        }
    }
    log.info(`Found ${enabledFeatures.length} enabled features.`);
    // Build the catalog of all possible features
    const featureCatalog = await buildFeatureCatalog(projectRoot);
    const includeDirs = [
        path.join(projectRoot, 'src'),
        path.join(projectRoot, 'lib/polymech-base/src')
    ];
    const classNameToFeatureMap = await buildClassNameToFeatureMap(featureCatalog, includeDirs);
    const finalTree = {};
    for (const feature of enabledFeatures) {
        const headerFile = featureCatalog.get(feature);
        if (!headerFile) {
            log.warn(`Enabled feature "${feature}" not found in features.h catalog.`);
            continue;
        }
        const dependencies = await analyzeComponentDependencies(headerFile, includeDirs, classNameToFeatureMap);
        const settings = {};
        const baseName = feature.replace('ENABLE_', '');
        for (const [define, { value, isComment }] of allDefines.entries()) {
            if (!isComment && define.startsWith(baseName + '_') && value) {
                settings[define] = value;
            }
        }
        finalTree[feature] = {
            headerFile,
            dependencies,
            settings
        };
    }
    return finalTree;
}
async function* getFiles(dir) {
    const dirents = await fs.readdir(dir, { withFileTypes: true });
    for (const dirent of dirents) {
        const res = path.resolve(dir, dirent.name);
        if (dirent.isDirectory()) {
            yield* getFiles(res);
        }
        else if (res.endsWith('.h')) {
            yield res;
        }
    }
}
export async function findComponents(includeDirs) {
    const componentMap = new Map();
    const parser = new Parser();
    parser.setLanguage(Cpp);
    const childByType = (node, type) => {
        for (const child of node.children) {
            if (child.grammarType === type) {
                return child;
            }
        }
    };
    const childrenByType = (node, type) => {
        const children = [];
        for (const child of node.children) {
            if (child.grammarType === type) {
                children.push(child);
            }
        }
    };
    for (const dir of includeDirs) {
        for await (const f of getFiles(dir)) {
            try {
                const content = await fs.readFile(f, 'utf-8');
                if (!content)
                    continue;
                const tree = parser.parse(content);
                const classNodes = tree.rootNode.descendantsOfType('class_specifier');
                const classNode = classNodes[0];
                if (!classNode)
                    continue;
                const nameNode = childByType(classNode, 'type_identifier');
                if (!nameNode)
                    continue;
                // componentMap.set(nameNode.text, f);
                for (const classNode of classNodes) {
                    const baseClauseNode = childByType(classNode, 'base_class_clause');
                    if (!baseClauseNode)
                        continue;
                    const nameNode = childByType(baseClauseNode, 'identifier');
                    if (!nameNode)
                        continue;
                    if (nameNode.text === 'Component') {
                        const nameNode = childByType(baseClauseNode, 'identifier');
                        if (!nameNode)
                            continue;
                        componentMap.set(nameNode.text, f);
                    }
                }
            }
            catch (error) {
                log.warn(`Skipping file: ${f}`, error instanceof Error ? error.message : error);
            }
        }
    }
    log.info(`Discovered ${componentMap.size} components inheriting from 'Component'.`);
    return componentMap;
}
async function getDependencies(classNode, className, componentMap) {
    const dependencies = [];
    const constructorNode = classNode.descendantsOfType('declaration').find(decl => decl.childForFieldName('declarator')?.childForFieldName('declarator')?.text === className);
    if (constructorNode) {
        const params = constructorNode.descendantsOfType('parameter_declaration');
        for (const param of params) {
            const typeNode = param.childForFieldName('type');
            const typeName = typeNode?.text.replace('*', '').trim();
            if (typeName && componentMap.has(typeName)) {
                const headerFile = componentMap.get(typeName);
                const depTree = await getComponentTree(typeName, componentMap);
                if (depTree)
                    dependencies.push(depTree);
            }
        }
    }
    return Array.from(new Map(dependencies.map(item => [item.name, item])).values());
}
export async function getComponentTree(targetComponentName, allComponents) {
    const headerFile = allComponents.get(targetComponentName);
    if (!headerFile) {
        log.warn(`Component "${targetComponentName}" not found in component map.`);
        return null;
    }
    const parser = new Parser();
    parser.setLanguage(Cpp);
    const content = await fs.readFile(headerFile, 'utf-8');
    const tree = parser.parse(content);
    const classNode = tree.rootNode.descendantsOfType('class_specifier')
        .find(node => node.childForFieldName('name')?.text === targetComponentName);
    if (!classNode) {
        log.warn(`Could not find class specifier for "${targetComponentName}" in ${headerFile}`);
        return null;
    }
    const baseComponent = classNode.childForFieldName('base')?.text.replace('public', '').trim() ?? null;
    const dependencies = await getDependencies(classNode, targetComponentName, allComponents);
    return {
        name: targetComponentName,
        headerFile: path.relative(path.resolve(headerFile, '..', '..', '..'), headerFile).replace(/\\/g, '/'),
        baseComponent,
        dependencies
    };
}
//# sourceMappingURL=di.js.map