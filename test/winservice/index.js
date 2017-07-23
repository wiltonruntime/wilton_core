
define([
    "lodash/string",
    "wilton/Logger",
    "wilton/loader",
    "wilton/misc",
    "wilton/thread"
], function(template, Logger, loader, misc, thread) {
    return {
        main: function() {
            // load config
            var conf = loader.loadModuleJson("winservice/config.json"); 
            var fpTmpl = conf.logging.appenders[0].filePath;
            string.templateSettings.interpolate = /{{([\s\S]+?)}}/g; 
            string.template(fpTmpl)({});



            print("service start called"); 
        },
    };
});
