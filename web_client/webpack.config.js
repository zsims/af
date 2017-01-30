const HtmlWebpackPlugin = require('html-webpack-plugin');
const webpack = require('webpack');

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
        filename: "app.js",
        path: __dirname + "/dist"
    },
    // Enable sourcemaps for debugging webpack's output.
    devtool: "source-map",
    devServer: {
        // Allow inline refreshing on changes
        inline: true,
        port: 3000,
        contentBase: __dirname + "/dist"
    },
    resolve: {
        // Add '.ts' and '.tsx' as resolvable extensions.
        extensions: ["", ".webpack.js", ".web.js", ".ts", ".tsx", ".js"]
    },
    module: {
        loaders: [
            // All files with a '.ts' or '.tsx' extension will be handled by 'awesome-typescript-loader'.
            { test: /\.tsx?$/, loader: "awesome-typescript-loader" },
            // Allow embedded CSS per https://webpack.github.io/docs/stylesheets.html
            { test: /\.css$/, loader: "style-loader!css-loader" }
        ],
        preLoaders: [
            // All output '.js' files will have any sourcemaps re-processed by 'source-map-loader'.
            { test: /\.js$/, loader: "source-map-loader" }
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