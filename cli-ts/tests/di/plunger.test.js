import { describe, it, expect } from 'vitest';
import { findComponents, getComponentTree } from '../../src/lib/di.js';
import path from 'path';
import fs from 'fs/promises';
describe('Component Tree Analysis', () => {
    it('should generate a complete and correct dependency tree for the Plunger component', async () => {
        const projectRoot = path.resolve(process.cwd(), '..', 'cassandra-rc2');
        const includeDirs = [
            path.join(projectRoot, 'src'),
            path.join(projectRoot, 'lib/polymech-base/src')
        ];
        // 1. Discover all valid components in the project that inherit from "Component"
        const allComponents = await findComponents(includeDirs);
        const outputDir = path.join(projectRoot, 'cli-ts', 'tests', 'ast');
        await fs.mkdir(outputDir, { recursive: true });
        // Save the discovered components map for inspection
        const componentsObject = Object.fromEntries(allComponents);
        await fs.writeFile(path.join(outputDir, 'components.json'), JSON.stringify(componentsObject, null, 2));
        expect(allComponents.has('Plunger')).toBe(true);
        expect(allComponents.has('SAKO_VFD')).toBe(true);
        expect(allComponents.has('Joystick')).toBe(true);
        expect(allComponents.has('POT')).toBe(true);
        // 2. Get the specific dependency tree for Plunger
        const plungerTree = await getComponentTree('Plunger', allComponents);
        expect(plungerTree).toBeDefined();
        // 3. Save the output for inspection
        const outputPath = path.join(outputDir, 'plunger.json');
        await fs.writeFile(outputPath, JSON.stringify(plungerTree, null, 2));
        // 4. Assert the structure is correct
        expect(plungerTree?.name).toEqual('Plunger');
        expect(plungerTree?.baseComponent).toEqual('Component');
        const dependencyNames = plungerTree?.dependencies.map(d => d.name);
        expect(dependencyNames).toContain('SAKO_VFD');
        expect(dependencyNames).toContain('Joystick');
        expect(dependencyNames).toContain('POT');
        const potDependency = plungerTree?.dependencies.find(d => d.name === 'POT');
        expect(potDependency).toBeDefined();
        expect(potDependency?.headerFile).toContain('components/POT.h');
    });
});
//# sourceMappingURL=plunger.test.js.map