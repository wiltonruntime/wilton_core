/* 
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

define([], function() {
    "use strict";

    return {
        main: function() {
            require([
                "wilton/dyload",
                "wilton/misc",
                "wilton/test/core/index",
                "wilton/test/db/index"
            ], function(dyload, misc, coreTests, dbTests) {
                coreTests.main();
                dyload({
                    name: "wilton_db",
                    directory: misc.getWiltonConfig().applicationDirectory + "../../../build/bin"
                });
                dbTests.main();
            });
        }
    };
});
