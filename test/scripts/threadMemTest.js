
define([
    "module",
    "lodash/collection",
    "wilton/Channel",
    "wilton/Logger",
    "wilton/thread"
], function(module, collection, Channel, Logger, thread) {
    var logger = new Logger(module.id);
    
    return {
        main: function(count) {
            Logger.initConsole("INFO");
            var chan = new Channel("threadMemTest");
            for (var i = 0; i < count; i++) {
                thread.run({
                    callbackScript: {
                        module: module.id,
                        func: "run",
                        args: [i + 1]
                    }
                });
            }
            thread.sleepMillis(4000);
            for (var i = 0; i < count; i++) {
                chan.send(true);
            }
            logger.info("main exit");
        },

        run: function(idx) {
            logger.info("thread started, idx: [" + idx + "]");
            Channel.lookup("threadMemTest").receive();
            logger.info("thread exit, idx: [" + idx + "]");
        }
    };
});
