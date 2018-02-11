/*
 * Copyright 2017, alex at staticlibs.net
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

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
