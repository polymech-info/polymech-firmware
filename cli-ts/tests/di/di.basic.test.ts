import { describe, it, expect, beforeAll } from 'vitest';
import { 
    buildFeatureCatalog, 
    buildClassNameToFeatureMap,
    analyzeComponentDependencies 
} from '../../src/lib/di.js';
import path from 'path';

describe('Dependency Injection Analysis - Basic Units', () => {

    let projectRoot: string;
    let featureCatalog: Map<string, string>;
    let classNameToFeatureMap: Map<string, string>;
    let includeDirs: string[];

    beforeAll(async () => {
        projectRoot = path.resolve(process.cwd(), '..', 'cassandra-rc2');
        includeDirs = [
            path.join(projectRoot, 'src'),
            path.join(projectRoot, 'lib/polymech-base/src')
        ];
    });

    it('Step 1: should build the feature catalog correctly', async () => {
        featureCatalog = await buildFeatureCatalog(projectRoot);
        expect(featureCatalog.size).toBeGreaterThan(10); // Should find many features
        expect(featureCatalog.get('ENABLE_PLUNGER')).toEqual('components/Plunger.h');
        expect(featureCatalog.get('ENABLE_SAKO_VFD')).toEqual('components/SAKO_VFD.h');
    });

    it('Step 2: should map class names to feature flags correctly', async () => {
        // Depends on Step 1
        featureCatalog = await buildFeatureCatalog(projectRoot);
        classNameToFeatureMap = await buildClassNameToFeatureMap(featureCatalog, includeDirs);

        expect(classNameToFeatureMap.size).toBeGreaterThan(10);
        expect(classNameToFeatureMap.get('Plunger')).toEqual('ENABLE_PLUNGER');
        expect(classNameToFeatureMap.get('SAKO_VFD')).toEqual('ENABLE_SAKO_VFD');
        expect(classNameToFeatureMap.get('Joystick')).toEqual('ENABLE_JOYSTICK');
        expect(classNameToFeatureMap.get('POT')).toEqual('ENABLE_POT');
    });

    it('Step 3: should analyze Plunger dependencies correctly', async () => {
        // Depends on Steps 1 & 2
        featureCatalog = await buildFeatureCatalog(projectRoot);
        classNameToFeatureMap = await buildClassNameToFeatureMap(featureCatalog, includeDirs);
        const plungerHeader = featureCatalog.get('ENABLE_PLUNGER');

        expect(plungerHeader).toBeDefined();

        const dependencies = await analyzeComponentDependencies(plungerHeader!, includeDirs, classNameToFeatureMap);
        
        expect(dependencies).toBeDefined();
        expect(dependencies.length).toBeGreaterThan(0);
        expect(dependencies).toContain('ENABLE_SAKO_VFD');
        expect(dependencies).toContain('ENABLE_JOYSTICK');
        expect(dependencies).toContain('ENABLE_POT');
    });
}); 