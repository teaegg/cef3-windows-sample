var app;
if (!app)
    app = {};
if(!app.window)
       app.window = {};
(function () {
    native function sendMessage();
    app.sendMessage = function(name, arguments) {
        return sendMessage(name, arguments);
    };
    
    native function WindowShow();
    app.window.show = function() {
        WindowShow();
    }
    
    native function WindowHide();
    app.window.hide = function() {
        WindowHide();
    }

    native function WindowMinimize();    
    app.window.minimize = function() {
        WindowMinimize();
    }
    
    native function WindowMaximize();
    app.window.maximize = function() {
        WindowMaximize();
    }
    
    native function WindowClose();
    app.window.close = function() {
        WindowClose();
    }
})();