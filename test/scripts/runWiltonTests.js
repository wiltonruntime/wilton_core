/* 
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

define([], function() {
    "use strict";

    return {
        main: function() {
            require(["wilton/dyload", "wilton/test/core/index", "wilton/test/db/index"], function(dyload, coreTests, dbTests) {
                coreTests.main();
                dyload({
                    path: "libwilton_db.so"
                }, function(){ /* quiet */ });
                dyload({
                    path: "wilton_db.dll"
                }, function(){ /* quiet */ });
                dbTests.main();
            });
        }
    };
});
