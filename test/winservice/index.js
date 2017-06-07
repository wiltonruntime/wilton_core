
define([], function() {
    return {
        start: function() {
           print("service start called"); 
        },

        stop: function() {
            print("service stop called");
        }
    };
});
