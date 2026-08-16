import path from 'path';
import { fileURLToPath } from 'url';
import webpack from 'webpack';

const __dirname = fileURLToPath(new URL('.', import.meta.url));

export default {
  devtool: false,
  plugins: [
    new webpack.BannerPlugin({ banner: "#!/usr/bin/env node", raw: true })
  ],
  entry: './dist-in/index.js',
  target: 'node',
  mode: 'production',
  module: {
    rules: []
  },
  optimization: {
    minimize: false
  },
  resolve: {
    extensions: ['.js', '.ts']
  },
  output: {
    filename: 'main_node.cjs',
    path: path.resolve(__dirname, 'dist')
  },
  externals: {
    'utf-8-validate': 'commonjs utf-8-validate',
    'bufferutil': 'commonjs bufferutil'
  }
};