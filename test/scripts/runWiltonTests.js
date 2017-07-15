/* 
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

define([], function() {
    "use strict";

    return {
        main: function() {
            require(["wilton/test/core/index"], function(tests) {
                tests.main();
            });
        }
    };
});
