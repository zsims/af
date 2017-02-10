const nodeExternals = require("webpack-node-externals");

module.exports = {
    target: "node",
    devtool: "source-map",
    externals: [nodeExternals()],
    resolve: {
        extensions: [".ts", ".tsx", ".js", ".jsx"]
    },
    output: {
        // sourcemap support for IntelliJ/Webstorm/VS Code
        devtoolModuleFilenameTemplate: '[absolute-resource-path]',
        devtoolFallbackModuleFilenameTemplate: '[absolute-resource-path]?[hash]'
    },
    module: {
        rules: [
            {
                test: /\.tsx?$/,
                use: "awesome-typescript-loader",
            },
            {
                test: /\.css$/,
                use: "ignore-loader"
            }
        ]
    }
};