
define(["wilton/thread"], function(thread) {
    return {
        main: function() {
            print("hi!");
            thread.waitForSignal();
            print("bye!");
        }
    };
});
