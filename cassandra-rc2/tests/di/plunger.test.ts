import { describe, it, expect } from 'vitest';
import { findComponents, getComponentTree } from '../../src/lib/di';
import * as path from 'path';
import { Plunger } from '../../src/lib/components/plunger';

describe('DI Basic Tests', () => {
    it('should correctly parse the component tree for Plunger', () => {
        const di_config = "tests/di";
        const components = findComponents(di_config);
        const plungerComponent = components.find(c => c.name === 'Plunger');
        expect(plungerComponent).toBeDefined();

        if (plungerComponent) {
            const plungerTree = getComponentTree(plungerComponent, components);
            
            // Basic checks for Plunger
            expect(plungerTree.name).toBe('Plunger');
            expect(plungerTree.dependencies).toHaveLength(4);

            // Check dependency names
            const dependencyNames = plungerTree.dependencies.map(d => d.name);
            expect(dependencyNames).toContain('SAKO_VFD');
            expect(dependencyNames).toContain('Joystick');
            expect(dependencyNames).toContain('POT');
            expect(dependencyNames).toContain('Logger');

            // Check for POT component details
            const potDependency = plungerTree.dependencies.find(d => d.name === 'POT');
            expect(potDependency).toBeDefined();
        }
    });
}); 