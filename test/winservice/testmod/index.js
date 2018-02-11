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
