import { describe, it, expect } from 'vitest';
import { findComponents } from '../../src/lib/di.js';
import path from 'path';
import fs from 'fs/promises';

describe('Component Discovery', () => {

    it('should scan the project and produce a map of all valid components', async () => {
        const projectRoot = path.resolve(process.cwd(), '..', 'cassandra-rc2');
        const includeDirs = [
            path.join(projectRoot, 'src'),
            path.join(projectRoot, 'lib/polymech-base/src')
        ];
        
        // This function scans all .h files and returns a map of ClassName -> AbsoluteFilePath
        const allComponents = await findComponents(includeDirs);
        
        // Save the output for inspection
        const outputDir = path.join(process.cwd(), 'tests', 'ast');
        await fs.mkdir(outputDir, { recursive: true });
        const outputPath = path.join(outputDir, 'components.json');
        
        // Convert Map to a plain object for pretty JSON serialization
        const componentsObject = Object.fromEntries(allComponents);
        await fs.writeFile(outputPath, JSON.stringify(componentsObject, null, 2));
        
        // Assert that the discovery is working at a basic level
        expect(allComponents).toBeDefined();
        expect(allComponents.size).toBeGreaterThan(10); // Expect to find a reasonable number of components
        expect(allComponents.has('Plunger')).toBe(true);
        expect(allComponents.get('Plunger')).toContain('Plunger.h');
    });
}); 