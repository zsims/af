const HtmlWebpackPlugin = require('html-webpack-plugin');
const webpack = require('webpack');
const path = require('path');

function isExternal(module) {
  var userRequest = module.userRequest;

  if (typeof userRequest !== 'string') {
    return false;
  }

  return userRequest.indexOf('node_modules') >= 0;
}

module.exports = {
    entry: {
        "app": "./src/index.tsx"
    },
    output: {
        filename: "[name].bundle.js",
        sourceMapFilename: "[name].bundle.map",
        path: path.resolve(__dirname, "./dist")
    },
    // Enable sourcemaps for debugging webpack's output.
    devtool: "source-map",
    devServer: {
        // Allow inline refreshing on changes
        inline: true,
        port: 3000,
        contentBase: path.resolve(__dirname, "./dist")
    },
    resolve: {
        extensions: [".ts", ".tsx", ".js", ".jsx"]
    },
    module: {
        rules: [
            {
                enforce: 'pre',
                test: /\.js$/,
                use: "source-map-loader"
            },
            {
                test: /\.tsx?$/,
                use: "awesome-typescript-loader",
            },
            {
                test: /\.css$/,
                use: ["style-loader", "css-loader"]
            }
        ]
    },
    plugins: [
        new webpack.optimize.CommonsChunkPlugin({
            name: 'vendor',
            filename: 'vendor.bundle.js',
            minChunks: function (module) {
                return isExternal(module);
            }
        }),
        new HtmlWebpackPlugin({ template: './src/index.html' })
    ]
};