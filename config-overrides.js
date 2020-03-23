const {
  override,
  fixBabelImports,
  addLessLoader,
  addWebpackPlugin,
  setWebpackTarget
} = require('customize-cra');
const AntdDayjsWebpackPlugin = require('antd-dayjs-webpack-plugin');

module.exports = override(
  fixBabelImports('antd', {
    libraryDirectory: 'es',
    style: true
  }),
  fixBabelImports('lodash', {
    libraryDirectory: '',
    camel2DashComponentName: false
  }),
  addLessLoader({
    javascriptEnabled: true
  }),
  addWebpackPlugin(new AntdDayjsWebpackPlugin()),
  setWebpackTarget('electron-renderer')
);
