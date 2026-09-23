mergeInto(LibraryManager.library, {
  cmc_js_emit: function (linePtr) {
    if (Module.cmcEmit) Module.cmcEmit(UTF8ToString(linePtr));
  },

  cmc_js_draw: function (opPtr, a, b, c, d, textPtr) {
    if (!Module.cmcDraw) return;
    Module.cmcDraw(UTF8ToString(opPtr), a, b, c, d, textPtr ? UTF8ToString(textPtr) : "");
  },

  cmc_js_should_stop: function () {
    return Module.cmcShouldStop && Module.cmcShouldStop() ? 1 : 0;
  },

  cmc_js_done: function () {
    if (Module.cmcDone) Module.cmcDone();
  },

  cmc_js_ask_async__deps: ['$Asyncify'],
  cmc_js_ask_async: function (promptPtr, bufferPtr, capacity) {
    var prompt = UTF8ToString(promptPtr);
    return Asyncify.handleAsync(function () {
      return new Promise(function (resolve) {
        var finish = function (answer) {
          var text = answer === null || answer === undefined ? "" : String(answer);
          stringToUTF8(text, bufferPtr, capacity);
          resolve();
        };
        if (Module.cmcAsk) Module.cmcAsk(prompt, finish);
        else finish("");
      });
    });
  },
});
