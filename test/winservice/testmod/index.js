
define([
    "wilton/Logger",
    "wilton/loader"
], function(Logger, loader) {

    return {
        main: function(cmd) {
            if ("start" === cmd) {
                var conf = loader.loadAppConfig(); 
                Logger.initialize(conf.logging);

                var logger = new Logger("winservice");
                logger.info("service start called"); 
            } else if ("stop" === cmd) {
                var logger = new Logger("winservice");
                logger.info("service stop called"); 
            }
        }
    };
});
