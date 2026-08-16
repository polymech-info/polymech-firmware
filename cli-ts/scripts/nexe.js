// nexe.js - Compile pm-media to Windows executable using nexe Node.js API
import { compile } from 'nexe';
import path from 'path';
import fs from 'fs';

async function buildExecutable() {    
    const outputDir = './dist/win-64';
    const outputFile = 'pm-fw-cli.exe';
    const entryPoint = './dist/main_node.cjs';
    const nexeTemp = '../../nexe-24';
    const nodeVersion = '24.5.0';
    
    // Ensure output directory exists
    if (!fs.existsSync(outputDir)) {
        fs.mkdirSync(outputDir, { recursive: true });
        console.log(`📁 Created output directory: ${outputDir}`);
    }
    
    // Ensure nexe temp directory exists
    if (!fs.existsSync(nexeTemp)) {
        fs.mkdirSync(nexeTemp, { recursive: true });
        console.log(`📁 Created temp directory: ${nexeTemp}`);
    }
    
    // Check if entry point exists
    if (!fs.existsSync(entryPoint)) {
        console.log(`❌ Entry point ${entryPoint} not found. Please run 'npm run build' first.`);
        process.exit(1);
    }
    
    const outputPath = path.join(outputDir, outputFile);
    
    console.log('📦 Compiling with nexe...');
    console.log(`   Entry: ${entryPoint}`);
    console.log(`   Output: ${outputPath}`);
    console.log(`   Temp: ${nexeTemp}`);
    console.log(`   Target: windows-x64-${nodeVersion}`);
    
    try {
        await compile({
            input: entryPoint,
            output: outputPath,
            target: `windows-x64-${nodeVersion}`,
            build: true,  // Build from source for native modules like sharp
            temp: nexeTemp,
            clean: false,
            name: 'pm-media',
            configure: ['--with-intl=full-icu'],  // Full ICU support
            make: ['-j4'],  // Parallel build
            loglevel: 'verbose',
            // Resources - include any additional files if needed
            resources: [
                // Add any resource patterns here if needed
                // './assets/**/*'
            ],
            patches: [
                // Patch for better native module support
                async (compiler, next) => {
                    // This patch helps with native modules like sharp
                    await compiler.replaceInFileAsync(
                        'lib/internal/bootstrap/pre_execution.js',
                        'process.dlopen = function(',
                        `
                        // Nexe patch for native modules
                        const originalDlopen = process.dlopen;
                        process.dlopen = function(`
                    );
                    return next();
                }
            ]
        });
        
        console.log(`✅ Successfully compiled to ${outputPath}`);
        
        // Show file size
        if (fs.existsSync(outputPath)) {
            const stats = fs.statSync(outputPath);
            const fileSizeInMB = (stats.size / (1024 * 1024)).toFixed(2);
            console.log(`📊 Executable size: ${fileSizeInMB} MB`);
        }
        
        console.log('🎉 Build complete!');
        
    } catch (error) {
        console.error('❌ Compilation failed:', error.message);
        if (error.stack) {
            console.error(error.stack);
        }
        process.exit(1);
    }
}

// Run the build
buildExecutable().catch(console.error);
