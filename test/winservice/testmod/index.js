
define([
    "wilton/Logger",
    "wilton/loader",
    "wilton/thread"
], function(Logger, loader, thread) {
    return {
        main: function() {
            var conf = loader.loadAppConfig(); 
            Logger.initialize(conf.logging);
            var logger = new Logger("winservice");
            logger.info("service start called"); 
            thread.waitForSignal();
            logger.info("service stop called"); 
        }
    };
});
