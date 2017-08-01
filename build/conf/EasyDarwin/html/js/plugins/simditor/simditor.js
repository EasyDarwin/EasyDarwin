(function (root, factory) {
  if (typeof define === 'function' && define.amd) {
    // AMD. Register as an anonymous module.
    define('simditor', ["jquery",
      "simple-module",
      "simple-hotkeys",
      "simple-uploader"], function ($, SimpleModule, simpleHotkeys, simpleUploader) {
      return (root.returnExportsGlobal = factory($, SimpleModule, simpleHotkeys, simpleUploader));
    });
  } else if (typeof exports === 'object') {
    // Node. Does not work with strict CommonJS, but
    // only CommonJS-like enviroments that support module.exports,
    // like Node.
    module.exports = factory(require("jquery"),
      require("simple-module"),
      require("simple-hotkeys"),
      require("simple-uploader"));
  } else {
    root['Simditor'] = factory(jQuery,
      SimpleModule,
      simple.hotkeys,
      simple.uploader);
  }
}(this, function ($, SimpleModule, simpleHotkeys, simpleUploader) {

var Selection,
  __hasProp = {}.hasOwnProperty,
  __extends = function(child, parent) { for (var key in parent) { if (__hasProp.call(parent, key)) child[key] = parent[key]; } function ctor() { this.constructor = child; } ctor.prototype = parent.prototype; child.prototype = new ctor(); child.__super__ = parent.prototype; return child; };

Selection = (function(_super) {
  __extends(Selection, _super);

  function Selection() {
    return Selection.__super__.constructor.apply(this, arguments);
  }

  Selection.pluginName = 'Selection';

  Selection.prototype._init = function() {
    this.editor = this._module;
    return this.sel = document.getSelection();
  };

  Selection.prototype.clear = function() {
    var e;
    try {
      return this.sel.removeAllRanges();
    } catch (_error) {
      e = _error;
    }
  };

  Selection.prototype.getRange = function() {
    if (!this.editor.inputManager.focused || !this.sel.rangeCount) {
      return null;
    }
    return this.sel.getRangeAt(0);
  };

  Selection.prototype.selectRange = function(range) {
    this.clear();
    this.sel.addRange(range);
    if (!this.editor.inputManager.focused && (this.editor.util.browser.firefox || this.editor.util.browser.msie)) {
      this.editor.body.focus();
    }
    return range;
  };

  Selection.prototype.rangeAtEndOf = function(node, range) {
    var endNode, endNodeLength, result;
    if (range == null) {
      range = this.getRange();
    }
    if (!((range != null) && range.collapsed)) {
      return;
    }
    node = $(node)[0];
    endNode = range.endContainer;
    endNodeLength = this.editor.util.getNodeLength(endNode);
    if (!(range.endOffset === endNodeLength - 1 && $(endNode).contents().last().is('br')) && range.endOffset !== endNodeLength) {
      return false;
    }
    if (node === endNode) {
      return true;
    } else if (!$.contains(node, endNode)) {
      return false;
    }
    result = true;
    $(endNode).parentsUntil(node).addBack().each((function(_this) {
      return function(i, n) {
        var $lastChild, nodes;
        nodes = $(n).parent().contents().filter(function() {
          return !(this !== n && this.nodeType === 3 && !this.nodeValue);
        });
        $lastChild = nodes.last();
        if (!($lastChild.get(0) === n || ($lastChild.is('br') && $lastChild.prev().get(0) === n))) {
          result = false;
          return false;
        }
      };
    })(this));
    return result;
  };

  Selection.prototype.rangeAtStartOf = function(node, range) {
    var result, startNode;
    if (range == null) {
      range = this.getRange();
    }
    if (!((range != null) && range.collapsed)) {
      return;
    }
    node = $(node)[0];
    startNode = range.startContainer;
    if (range.startOffset !== 0) {
      return false;
    }
    if (node === startNode) {
      return true;
    } else if (!$.contains(node, startNode)) {
      return false;
    }
    result = true;
    $(startNode).parentsUntil(node).addBack().each((function(_this) {
      return function(i, n) {
        var nodes;
        nodes = $(n).parent().contents().filter(function() {
          return !(this !== n && this.nodeType === 3 && !this.nodeValue);
        });
        if (nodes.first().get(0) !== n) {
          return result = false;
        }
      };
    })(this));
    return result;
  };

  Selection.prototype.insertNode = function(node, range) {
    if (range == null) {
      range = this.getRange();
    }
    if (range == null) {
      return;
    }
    node = $(node)[0];
    range.insertNode(node);
    return this.setRangeAfter(node, range);
  };

  Selection.prototype.setRangeAfter = function(node, range) {
    if (range == null) {
      range = this.getRange();
    }
    if (range == null) {
      return;
    }
    node = $(node)[0];
    range.setEndAfter(node);
    range.collapse(false);
    return this.selectRange(range);
  };

  Selection.prototype.setRangeBefore = function(node, range) {
    if (range == null) {
      range = this.getRange();
    }
    if (range == null) {
      return;
    }
    node = $(node)[0];
    range.setEndBefore(node);
    range.collapse(false);
    return this.selectRange(range);
  };

  Selection.prototype.setRangeAtStartOf = function(node, range) {
    if (range == null) {
      range = this.getRange();
    }
    node = $(node).get(0);
    range.setEnd(node, 0);
    range.collapse(false);
    return this.selectRange(range);
  };

  Selection.prototype.setRangeAtEndOf = function(node, range) {
    var $lastNode, $node, contents, lastChild, lastText, nodeLength;
    if (range == null) {
      range = this.getRange();
    }
    $node = $(node);
    node = $node.get(0);
    if ($node.is('pre')) {
      contents = $node.contents();
      if (contents.length > 0) {
        lastChild = contents.last();
        lastText = lastChild.text();
        if (lastText.charAt(lastText.length - 1) === '\n') {
          range.setEnd(lastChild[0], this.editor.util.getNodeLength(lastChild[0]) - 1);
        } else {
          range.setEnd(lastChild[0], this.editor.util.getNodeLength(lastChild[0]));
        }
      } else {
        range.setEnd(node, 0);
      }
    } else {
      nodeLength = this.editor.util.getNodeLength(node);
      if (node.nodeType !== 3 && nodeLength > 0) {
        $lastNode = $(node).contents().last();
        if ($lastNode.is('br')) {
          nodeLength -= 1;
        } else if ($lastNode[0].nodeType !== 3 && this.editor.util.isEmptyNode($lastNode)) {
          $lastNode.append(this.editor.util.phBr);
          node = $lastNode[0];
          nodeLength = 0;
        }
      }
      range.setEnd(node, nodeLength);
    }
    range.collapse(false);
    return this.selectRange(range);
  };

  Selection.prototype.deleteRangeContents = function(range) {
    var endRange, startRange;
    if (range == null) {
      range = this.getRange();
    }
    startRange = range.cloneRange();
    endRange = range.cloneRange();
    startRange.collapse(true);
    endRange.collapse(false);
    if (!range.collapsed && this.rangeAtStartOf(this.editor.body, startRange) && this.rangeAtEndOf(this.editor.body, endRange)) {
      this.editor.body.empty();
      range.setStart(this.editor.body[0], 0);
      range.collapse(true);
      this.selectRange(range);
    } else {
      range.deleteContents();
    }
    return range;
  };

  Selection.prototype.breakBlockEl = function(el, range) {
    var $el;
    if (range == null) {
      range = this.getRange();
    }
    $el = $(el);
    if (!range.collapsed) {
      return $el;
    }
    range.setStartBefore($el.get(0));
    if (range.collapsed) {
      return $el;
    }
    return $el.before(range.extractContents());
  };

  Selection.prototype.save = function(range) {
    var endCaret, endRange, startCaret;
    if (range == null) {
      range = this.getRange();
    }
    if (this._selectionSaved) {
      return;
    }
    endRange = range.cloneRange();
    endRange.collapse(false);
    startCaret = $('<span/>').addClass('simditor-caret-start');
    endCaret = $('<span/>').addClass('simditor-caret-end');
    endRange.insertNode(endCaret[0]);
    range.insertNode(startCaret[0]);
    this.clear();
    return this._selectionSaved = true;
  };

  Selection.prototype.restore = function() {
    var endCaret, endContainer, endOffset, range, startCaret, startContainer, startOffset;
    if (!this._selectionSaved) {
      return false;
    }
    startCaret = this.editor.body.find('.simditor-caret-start');
    endCaret = this.editor.body.find('.simditor-caret-end');
    if (startCaret.length && endCaret.length) {
      startContainer = startCaret.parent();
      startOffset = startContainer.contents().index(startCaret);
      endContainer = endCaret.parent();
      endOffset = endContainer.contents().index(endCaret);
      if (startContainer[0] === endContainer[0]) {
        endOffset -= 1;
      }
      range = document.createRange();
      range.setStart(startContainer.get(0), startOffset);
      range.setEnd(endContainer.get(0), endOffset);
      startCaret.remove();
      endCaret.remove();
      this.selectRange(range);
    } else {
      startCaret.remove();
      endCaret.remove();
    }
    this._selectionSaved = false;
    return range;
  };

  return Selection;

})(SimpleModule);

var Formatter,
  __hasProp = {}.hasOwnProperty,
  __extends = function(child, parent) { for (var key in parent) { if (__hasProp.call(parent, key)) child[key] = parent[key]; } function ctor() { this.constructor = child; } ctor.prototype = parent.prototype; child.prototype = new ctor(); child.__super__ = parent.prototype; return child; },
  __indexOf = [].indexOf || function(item) { for (var i = 0, l = this.length; i < l; i++) { if (i in this && this[i] === item) return i; } return -1; };

Formatter = (function(_super) {
  __extends(Formatter, _super);

  function Formatter() {
    return Formatter.__super__.constructor.apply(this, arguments);
  }

  Formatter.pluginName = 'Formatter';

  Formatter.prototype._init = function() {
    this.editor = this._module;
    this._allowedTags = ['br', 'a', 'img', 'b', 'strong', 'i', 'u', 'font', 'p', 'ul', 'ol', 'li', 'blockquote', 'pre', 'h1', 'h2', 'h3', 'h4', 'hr'];
    this._allowedAttributes = {
      img: ['src', 'alt', 'width', 'height', 'data-image-src', 'data-image-size', 'data-image-name', 'data-non-image'],
      a: ['href', 'target'],
      font: ['color'],
      pre: ['data-lang', 'class'],
      p: ['data-indent'],
      h1: ['data-indent'],
      h2: ['data-indent'],
      h3: ['data-indent'],
      h4: ['data-indent']
    };
    return this.editor.body.on('click', 'a', (function(_this) {
      return function(e) {
        return false;
      };
    })(this));
  };

  Formatter.prototype.decorate = function($el) {
    if ($el == null) {
      $el = this.editor.body;
    }
    return this.editor.trigger('decorate', [$el]);
  };

  Formatter.prototype.undecorate = function($el) {
    if ($el == null) {
      $el = this.editor.body.clone();
    }
    this.editor.trigger('undecorate', [$el]);
    return $.trim($el.html());
  };

  Formatter.prototype.autolink = function($el) {
    var $node, findLinkNode, lastIndex, linkNodes, match, re, replaceEls, text, uri, _i, _len;
    if ($el == null) {
      $el = this.editor.body;
    }
    linkNodes = [];
    findLinkNode = function($parentNode) {
      return $parentNode.contents().each(function(i, node) {
        var $node, text;
        $node = $(node);
        if ($node.is('a') || $node.closest('a, pre', $el).length) {
          return;
        }
        if ($node.contents().length) {
          return findLinkNode($node);
        } else if ((text = $node.text()) && /https?:\/\/|www\./ig.test(text)) {
          return linkNodes.push($node);
        }
      });
    };
    findLinkNode($el);
    re = /(https?:\/\/|www\.)[\w\-\.\?&=\/#%:,@\!\+]+/ig;
    for (_i = 0, _len = linkNodes.length; _i < _len; _i++) {
      $node = linkNodes[_i];
      text = $node.text();
      replaceEls = [];
      match = null;
      lastIndex = 0;
      while ((match = re.exec(text)) !== null) {
        replaceEls.push(document.createTextNode(text.substring(lastIndex, match.index)));
        lastIndex = re.lastIndex;
        uri = /^(http(s)?:\/\/|\/)/.test(match[0]) ? match[0] : 'http://' + match[0];
        replaceEls.push($('<a href="' + uri + '" rel="nofollow"></a>').text(match[0])[0]);
      }
      replaceEls.push(document.createTextNode(text.substring(lastIndex)));
      $node.replaceWith($(replaceEls));
    }
    return $el;
  };

  Formatter.prototype.format = function($el) {
    var $node, blockNode, n, node, _i, _j, _len, _len1, _ref, _ref1;
    if ($el == null) {
      $el = this.editor.body;
    }
    if ($el.is(':empty')) {
      $el.append('<p>' + this.editor.util.phBr + '</p>');
      return $el;
    }
    _ref = $el.contents();
    for (_i = 0, _len = _ref.length; _i < _len; _i++) {
      n = _ref[_i];
      this.cleanNode(n, true);
    }
    _ref1 = $el.contents();
    for (_j = 0, _len1 = _ref1.length; _j < _len1; _j++) {
      node = _ref1[_j];
      $node = $(node);
      if ($node.is('br')) {
        if (typeof blockNode !== "undefined" && blockNode !== null) {
          blockNode = null;
        }
        $node.remove();
      } else if (this.editor.util.isBlockNode(node)) {
        if ($node.is('li')) {
          if (blockNode && blockNode.is('ul, ol')) {
            blockNode.append(node);
          } else {
            blockNode = $('<ul/>').insertBefore(node);
            blockNode.append(node);
          }
        } else {
          blockNode = null;
        }
      } else {
        if (!blockNode || blockNode.is('ul, ol')) {
          blockNode = $('<p/>').insertBefore(node);
        }
        blockNode.append(node);
      }
    }
    return $el;
  };

  Formatter.prototype.cleanNode = function(node, recursive) {
    var $childImg, $node, $p, $td, allowedAttributes, attr, contents, isDecoration, n, text, textNode, _i, _j, _len, _len1, _ref, _ref1;
    $node = $(node);
    if (!($node.length > 0)) {
      return;
    }
    if ($node[0].nodeType === 3) {
      text = $node.text().replace(/(\r\n|\n|\r)/gm, '');
      if (text) {
        textNode = document.createTextNode(text);
        $node.replaceWith(textNode);
      } else {
        $node.remove();
      }
      return;
    }
    contents = $node.contents();
    isDecoration = $node.is('[class^="simditor-"]');
    if ($node.is(this._allowedTags.join(',')) || isDecoration) {
      if ($node.is('a') && ($childImg = $node.find('img')).length > 0) {
        $node.replaceWith($childImg);
        $node = $childImg;
        contents = null;
      }
      if ($node.is('img') && $node.hasClass('uploading')) {
        $node.remove();
      }
      if (!isDecoration) {
        allowedAttributes = this._allowedAttributes[$node[0].tagName.toLowerCase()];
        _ref = $.makeArray($node[0].attributes);
        for (_i = 0, _len = _ref.length; _i < _len; _i++) {
          attr = _ref[_i];
          if (!((allowedAttributes != null) && (_ref1 = attr.name, __indexOf.call(allowedAttributes, _ref1) >= 0))) {
            $node.removeAttr(attr.name);
          }
        }
      }
    } else if ($node[0].nodeType === 1 && !$node.is(':empty')) {
      if ($node.is('div, article, dl, header, footer, tr')) {
        $node.append('<br/>');
        contents.first().unwrap();
      } else if ($node.is('table')) {
        $p = $('<p/>');
        $node.find('tr').each((function(_this) {
          return function(i, tr) {
            return $p.append($(tr).text() + '<br/>');
          };
        })(this));
        $node.replaceWith($p);
        contents = null;
      } else if ($node.is('thead, tfoot')) {
        $node.remove();
        contents = null;
      } else if ($node.is('th')) {
        $td = $('<td/>').append($node.contents());
        $node.replaceWith($td);
      } else {
        contents.first().unwrap();
      }
    } else {
      $node.remove();
      contents = null;
    }
    if (recursive && (contents != null) && !$node.is('pre')) {
      for (_j = 0, _len1 = contents.length; _j < _len1; _j++) {
        n = contents[_j];
        this.cleanNode(n, true);
      }
    }
    return null;
  };

  Formatter.prototype.clearHtml = function(html, lineBreak) {
    var container, contents, result;
    if (lineBreak == null) {
      lineBreak = true;
    }
    container = $('<div/>').append(html);
    contents = container.contents();
    result = '';
    contents.each((function(_this) {
      return function(i, node) {
        var $node, children;
        if (node.nodeType === 3) {
          return result += node.nodeValue;
        } else if (node.nodeType === 1) {
          $node = $(node);
          children = $node.contents();
          if (children.length > 0) {
            result += _this.clearHtml(children);
          }
          if (lineBreak && i < contents.length - 1 && $node.is('br, p, div, li, tr, pre, address, artticle, aside, dl, figcaption, footer, h1, h2, h3, h4, header')) {
            return result += '\n';
          }
        }
      };
    })(this));
    return result;
  };

  Formatter.prototype.beautify = function($contents) {
    var uselessP;
    uselessP = function($el) {
      return !!($el.is('p') && !$el.text() && $el.children(':not(br)').length < 1);
    };
    return $contents.each((function(_this) {
      return function(i, el) {
        var $el;
        $el = $(el);
        if ($el.is(':not(img, br, col, td, hr, [class^="simditor-"]):empty')) {
          $el.remove();
        }
        if (uselessP($el)) {
          $el.remove();
        }
        return $el.find(':not(img, br, col, td, hr, [class^="simditor-"]):empty').remove();
      };
    })(this));
  };

  return Formatter;

})(SimpleModule);

var InputManager,
  __hasProp = {}.hasOwnProperty,
  __extends = function(child, parent) { for (var key in parent) { if (__hasProp.call(parent, key)) child[key] = parent[key]; } function ctor() { this.constructor = child; } ctor.prototype = parent.prototype; child.prototype = new ctor(); child.__super__ = parent.prototype; return child; },
  __indexOf = [].indexOf || function(item) { for (var i = 0, l = this.length; i < l; i++) { if (i in this && this[i] === item) return i; } return -1; };

InputManager = (function(_super) {
  __extends(InputManager, _super);

  function InputManager() {
    return InputManager.__super__.constructor.apply(this, arguments);
  }

  InputManager.pluginName = 'InputManager';

  InputManager.prototype.opts = {
    pasteImage: false
  };

  InputManager.prototype._modifierKeys = [16, 17, 18, 91, 93, 224];

  InputManager.prototype._arrowKeys = [37, 38, 39, 40];

  InputManager.prototype._init = function() {
    var submitKey;
    this.editor = this._module;
    if (this.opts.pasteImage && typeof this.opts.pasteImage !== 'string') {
      this.opts.pasteImage = 'inline';
    }
    this._keystrokeHandlers = {};
    this.hotkeys = simpleHotkeys({
      el: this.editor.body
    });
    this._pasteArea = $('<div/>').css({
      width: '1px',
      height: '1px',
      overflow: 'hidden',
      position: 'fixed',
      right: '0',
      bottom: '100px'
    }).attr({
      tabIndex: '-1',
      contentEditable: true
    }).addClass('simditor-paste-area').appendTo(this.editor.el);
    this._cleanPasteArea = $('<textarea/>').css({
      width: '1px',
      height: '1px',
      overflow: 'hidden',
      position: 'fixed',
      right: '0',
      bottom: '101px'
    }).attr({
      tabIndex: '-1'
    }).addClass('simditor-clean-paste-area').appendTo(this.editor.el);
    $(document).on('selectionchange.simditor' + this.editor.id, (function(_this) {
      return function(e) {
        if (!_this.focused) {
          return;
        }
        if (_this._selectionTimer) {
          clearTimeout(_this._selectionTimer);
          _this._selectionTimer = null;
        }
        return _this._selectionTimer = setTimeout(function() {
          return _this.editor.trigger('selectionchanged');
        }, 20);
      };
    })(this));
    this.editor.on('valuechanged', (function(_this) {
      return function() {
        if (!_this.editor.util.closestBlockEl() && _this.focused) {
          _this.editor.selection.save();
          _this.editor.formatter.format();
          _this.editor.selection.restore();
        }
        _this.editor.body.find('hr, pre, .simditor-table').each(function(i, el) {
          var $el, formatted;
          $el = $(el);
          if ($el.parent().is('blockquote') || $el.parent()[0] === _this.editor.body[0]) {
            formatted = false;
            if ($el.next().length === 0) {
              $('<p/>').append(_this.editor.util.phBr).insertAfter($el);
              formatted = true;
            }
            if ($el.prev().length === 0) {
              $('<p/>').append(_this.editor.util.phBr).insertBefore($el);
              formatted = true;
            }
            if (formatted) {
              return setTimeout(function() {
                return _this.editor.trigger('valuechanged');
              }, 10);
            }
          }
        });
        _this.editor.body.find('pre:empty').append(_this.editor.util.phBr);
        if (!_this.editor.util.supportSelectionChange && _this.focused) {
          return _this.editor.trigger('selectionchanged');
        }
      };
    })(this));
    this.editor.on('selectionchanged', (function(_this) {
      return function(e) {
        return _this.editor.undoManager.update();
      };
    })(this));
    this.editor.body.on('keydown', $.proxy(this._onKeyDown, this)).on('keypress', $.proxy(this._onKeyPress, this)).on('keyup', $.proxy(this._onKeyUp, this)).on('mouseup', $.proxy(this._onMouseUp, this)).on('focus', $.proxy(this._onFocus, this)).on('blur', $.proxy(this._onBlur, this)).on('paste', $.proxy(this._onPaste, this)).on('drop', $.proxy(this._onDrop, this));
    if (this.editor.util.browser.firefox) {
      this.addShortcut('cmd+left', (function(_this) {
        return function(e) {
          e.preventDefault();
          _this.editor.selection.sel.modify('move', 'backward', 'lineboundary');
          return false;
        };
      })(this));
      this.addShortcut('cmd+right', (function(_this) {
        return function(e) {
          e.preventDefault();
          _this.editor.selection.sel.modify('move', 'forward', 'lineboundary');
          return false;
        };
      })(this));
      this.addShortcut('cmd+a', (function(_this) {
        return function(e) {
          var $children, firstBlock, lastBlock, range;
          $children = _this.editor.body.children();
          if (!($children.length > 0)) {
            return;
          }
          firstBlock = $children.first().get(0);
          lastBlock = $children.last().get(0);
          range = document.createRange();
          range.setStart(firstBlock, 0);
          range.setEnd(lastBlock, _this.editor.util.getNodeLength(lastBlock));
          _this.editor.selection.selectRange(range);
          return false;
        };
      })(this));
    }
    submitKey = this.editor.util.os.mac ? 'cmd+enter' : 'ctrl+enter';
    this.addShortcut(submitKey, (function(_this) {
      return function(e) {
        _this.editor.el.closest('form').find('button:submit').click();
        return false;
      };
    })(this));
    if (this.editor.textarea.attr('autofocus')) {
      return setTimeout((function(_this) {
        return function() {
          return _this.editor.focus();
        };
      })(this), 0);
    }
  };

  InputManager.prototype._onFocus = function(e) {
    this.editor.el.addClass('focus').removeClass('error');
    this.focused = true;
    this.lastCaretPosition = null;
    return setTimeout((function(_this) {
      return function() {
        _this.editor.triggerHandler('focus');
        return _this.editor.trigger('selectionchanged');
      };
    })(this), 0);
  };

  InputManager.prototype._onBlur = function(e) {
    var _ref;
    this.editor.el.removeClass('focus');
    this.editor.sync();
    this.focused = false;
    this.lastCaretPosition = (_ref = this.editor.undoManager.currentState()) != null ? _ref.caret : void 0;
    return this.editor.triggerHandler('blur');
  };

  InputManager.prototype._onMouseUp = function(e) {
    if (!this.editor.util.supportSelectionChange) {
      return setTimeout((function(_this) {
        return function() {
          return _this.editor.trigger('selectionchanged');
        };
      })(this), 0);
    }
  };

  InputManager.prototype._onKeyDown = function(e) {
    var $blockEl, metaKey, result, _base, _ref, _ref1;
    if (this.editor.triggerHandler(e) === false) {
      return false;
    }
    if (this.hotkeys.respondTo(e)) {
      return;
    }
    if (e.which in this._keystrokeHandlers) {
      result = typeof (_base = this._keystrokeHandlers[e.which])['*'] === "function" ? _base['*'](e) : void 0;
      if (result) {
        this.editor.trigger('valuechanged');
        return false;
      }
      this.editor.util.traverseUp((function(_this) {
        return function(node) {
          var handler, _ref;
          if (node.nodeType !== 1) {
            return;
          }
          handler = (_ref = _this._keystrokeHandlers[e.which]) != null ? _ref[node.tagName.toLowerCase()] : void 0;
          result = typeof handler === "function" ? handler(e, $(node)) : void 0;
          if (result === true || result === false) {
            return false;
          }
        };
      })(this));
      if (result) {
        this.editor.trigger('valuechanged');
        return false;
      }
    }
    if ((_ref = e.which, __indexOf.call(this._modifierKeys, _ref) >= 0) || (_ref1 = e.which, __indexOf.call(this._arrowKeys, _ref1) >= 0)) {
      return;
    }
    metaKey = this.editor.util.metaKey(e);
    $blockEl = this.editor.util.closestBlockEl();
    if (metaKey && e.which === 86) {
      return;
    }
    if (this.editor.util.browser.webkit && e.which === 8 && this.editor.selection.rangeAtStartOf($blockEl)) {
      setTimeout((function(_this) {
        return function() {
          var $newBlockEl;
          if (!_this.focused) {
            return;
          }
          $newBlockEl = _this.editor.util.closestBlockEl();
          _this.editor.selection.save();
          _this.editor.formatter.cleanNode($newBlockEl, true);
          _this.editor.selection.restore();
          return _this.editor.trigger('valuechanged');
        };
      })(this), 10);
      this.typing = true;
    } else if (this._typing) {
      if (this._typing !== true) {
        clearTimeout(this._typing);
      }
      this._typing = setTimeout((function(_this) {
        return function() {
          _this.editor.trigger('valuechanged');
          return _this._typing = false;
        };
      })(this), 200);
    } else {
      setTimeout((function(_this) {
        return function() {
          return _this.editor.trigger('valuechanged');
        };
      })(this), 10);
      this._typing = true;
    }
    return null;
  };

  InputManager.prototype._onKeyPress = function(e) {
    if (this.editor.triggerHandler(e) === false) {
      return false;
    }
  };

  InputManager.prototype._onKeyUp = function(e) {
    var p, _ref;
    if (this.editor.triggerHandler(e) === false) {
      return false;
    }
    if (!this.editor.util.supportSelectionChange && (_ref = e.which, __indexOf.call(this._arrowKeys, _ref) >= 0)) {
      this.editor.trigger('selectionchanged');
      return;
    }
    if ((e.which === 8 || e.which === 46) && this.editor.util.isEmptyNode(this.editor.body)) {
      this.editor.body.empty();
      p = $('<p/>').append(this.editor.util.phBr).appendTo(this.editor.body);
      this.editor.selection.setRangeAtStartOf(p);
    }
  };

  InputManager.prototype._onPaste = function(e) {
    var $blockEl, cleanPaste, imageFile, pasteItem, range, uploadOpt, _ref;
    if (this.editor.triggerHandler(e) === false) {
      return false;
    }
    range = this.editor.selection.deleteRangeContents();
    if (!range.collapsed) {
      range.collapse(true);
    }
    $blockEl = this.editor.util.closestBlockEl();
    cleanPaste = $blockEl.is('pre, table');
    if (e.originalEvent.clipboardData && e.originalEvent.clipboardData.items && e.originalEvent.clipboardData.items.length > 0) {
      pasteItem = e.originalEvent.clipboardData.items[0];
      if (/^image\//.test(pasteItem.type) && !cleanPaste) {
        imageFile = pasteItem.getAsFile();
        if (!((imageFile != null) && this.opts.pasteImage)) {
          return;
        }
        if (!imageFile.name) {
          imageFile.name = "Clipboard Image.png";
        }
        uploadOpt = {};
        uploadOpt[this.opts.pasteImage] = true;
        if ((_ref = this.editor.uploader) != null) {
          _ref.upload(imageFile, uploadOpt);
        }
        return false;
      }
    }
    this.editor.selection.save(range);
    if (cleanPaste) {
      this._cleanPasteArea.focus();
      if (this.editor.util.browser.firefox) {
        e.preventDefault();
        this._cleanPasteArea.val(e.originalEvent.clipboardData.getData('text/plain'));
      } else if (this.editor.util.browser.msie && this.editor.util.browser.version === 10) {
        e.preventDefault();
        this._cleanPasteArea.val(window.clipboardData.getData('Text'));
      }
    } else {
      this._pasteArea.focus();
      if (this.editor.util.browser.msie && this.editor.util.browser.version === 10) {
        e.preventDefault();
        this._pasteArea.html(window.clipboardData.getData('Text'));
      }
    }
    return setTimeout((function(_this) {
      return function() {
        var $img, blob, children, insertPosition, lastLine, line, lines, node, pasteContent, _i, _j, _k, _l, _len, _len1, _len2, _len3, _len4, _m, _ref1, _ref2, _ref3;
        if (_this._pasteArea.is(':empty') && !_this._cleanPasteArea.val()) {
          pasteContent = null;
        } else if (cleanPaste) {
          pasteContent = _this._cleanPasteArea.val();
        } else {
          pasteContent = $('<div/>').append(_this._pasteArea.contents());
          pasteContent.find('table colgroup').remove();
          _this.editor.formatter.format(pasteContent);
          _this.editor.formatter.decorate(pasteContent);
          _this.editor.formatter.beautify(pasteContent.children());
          pasteContent = pasteContent.contents();
        }
        _this._pasteArea.empty();
        _this._cleanPasteArea.val('');
        range = _this.editor.selection.restore();
        if (_this.editor.triggerHandler('pasting', [pasteContent]) === false) {
          return;
        }
        if (!pasteContent) {
          return;
        } else if (cleanPaste) {
          if ($blockEl.is('table')) {
            lines = pasteContent.split('\n');
            lastLine = lines.pop();
            for (_i = 0, _len = lines.length; _i < _len; _i++) {
              line = lines[_i];
              _this.editor.selection.insertNode(document.createTextNode(line));
              _this.editor.selection.insertNode($('<br/>'));
            }
            _this.editor.selection.insertNode(document.createTextNode(lastLine));
          } else {
            pasteContent = $('<div/>').text(pasteContent);
            _ref1 = pasteContent.contents();
            for (_j = 0, _len1 = _ref1.length; _j < _len1; _j++) {
              node = _ref1[_j];
              _this.editor.selection.insertNode($(node)[0], range);
            }
          }
        } else if ($blockEl.is(_this.editor.body)) {
          for (_k = 0, _len2 = pasteContent.length; _k < _len2; _k++) {
            node = pasteContent[_k];
            _this.editor.selection.insertNode(node, range);
          }
        } else if (pasteContent.length < 1) {
          return;
        } else if (pasteContent.length === 1) {
          if (pasteContent.is('p')) {
            children = pasteContent.contents();
            if (children.length === 1 && children.is('img')) {
              $img = children;
              if (/^data:image/.test($img.attr('src'))) {
                if (!_this.opts.pasteImage) {
                  return;
                }
                blob = _this.editor.util.dataURLtoBlob($img.attr("src"));
                blob.name = "Clipboard Image.png";
                uploadOpt = {};
                uploadOpt[_this.opts.pasteImage] = true;
                if ((_ref2 = _this.editor.uploader) != null) {
                  _ref2.upload(blob, uploadOpt);
                }
                return;
              } else if ($img.is('img[src^="webkit-fake-url://"]')) {
                return;
              }
            }
            for (_l = 0, _len3 = children.length; _l < _len3; _l++) {
              node = children[_l];
              _this.editor.selection.insertNode(node, range);
            }
          } else if ($blockEl.is('p') && _this.editor.util.isEmptyNode($blockEl)) {
            $blockEl.replaceWith(pasteContent);
            _this.editor.selection.setRangeAtEndOf(pasteContent, range);
          } else if (pasteContent.is('ul, ol')) {
            if (pasteContent.find('li').length === 1) {
              pasteContent = $('<div/>').text(pasteContent.text());
              _ref3 = pasteContent.contents();
              for (_m = 0, _len4 = _ref3.length; _m < _len4; _m++) {
                node = _ref3[_m];
                _this.editor.selection.insertNode($(node)[0], range);
              }
            } else if ($blockEl.is('li')) {
              $blockEl.parent().after(pasteContent);
              _this.editor.selection.setRangeAtEndOf(pasteContent, range);
            } else {
              $blockEl.after(pasteContent);
              _this.editor.selection.setRangeAtEndOf(pasteContent, range);
            }
          } else {
            $blockEl.after(pasteContent);
            _this.editor.selection.setRangeAtEndOf(pasteContent, range);
          }
        } else {
          if ($blockEl.is('li')) {
            $blockEl = $blockEl.parent();
          }
          if (_this.editor.selection.rangeAtStartOf($blockEl, range)) {
            insertPosition = 'before';
          } else if (_this.editor.selection.rangeAtEndOf($blockEl, range)) {
            insertPosition = 'after';
          } else {
            _this.editor.selection.breakBlockEl($blockEl, range);
            insertPosition = 'before';
          }
          $blockEl[insertPosition](pasteContent);
          _this.editor.selection.setRangeAtEndOf(pasteContent.last(), range);
        }
        return _this.editor.trigger('valuechanged');
      };
    })(this), 10);
  };

  InputManager.prototype._onDrop = function(e) {
    if (this.editor.triggerHandler(e) === false) {
      return false;
    }
    return setTimeout((function(_this) {
      return function() {
        return _this.editor.trigger('valuechanged');
      };
    })(this), 0);
  };

  InputManager.prototype.addKeystrokeHandler = function(key, node, handler) {
    if (!this._keystrokeHandlers[key]) {
      this._keystrokeHandlers[key] = {};
    }
    return this._keystrokeHandlers[key][node] = handler;
  };

  InputManager.prototype.addShortcut = function(keys, handler) {
    return this.hotkeys.add(keys, $.proxy(handler, this));
  };

  return InputManager;

})(SimpleModule);

var Keystroke,
  __hasProp = {}.hasOwnProperty,
  __extends = function(child, parent) { for (var key in parent) { if (__hasProp.call(parent, key)) child[key] = parent[key]; } function ctor() { this.constructor = child; } ctor.prototype = parent.prototype; child.prototype = new ctor(); child.__super__ = parent.prototype; return child; };

Keystroke = (function(_super) {
  __extends(Keystroke, _super);

  function Keystroke() {
    return Keystroke.__super__.constructor.apply(this, arguments);
  }

  Keystroke.pluginName = 'Keystroke';

  Keystroke.prototype._init = function() {
    var titleEnterHandler;
    this.editor = this._module;
    if (this.editor.util.browser.safari) {
      this.editor.inputManager.addKeystrokeHandler('13', '*', (function(_this) {
        return function(e) {
          var $blockEl, $br;
          if (!e.shiftKey) {
            return;
          }
          $blockEl = _this.editor.util.closestBlockEl();
          if ($blockEl.is('pre')) {
            return;
          }
          $br = $('<br/>');
          if (_this.editor.selection.rangeAtEndOf($blockEl)) {
            _this.editor.selection.insertNode($br);
            _this.editor.selection.insertNode($('<br/>'));
            _this.editor.selection.setRangeBefore($br);
          } else {
            _this.editor.selection.insertNode($br);
          }
          return true;
        };
      })(this));
    }
    if (this.editor.util.browser.webkit || this.editor.util.browser.msie) {
      titleEnterHandler = (function(_this) {
        return function(e, $node) {
          var $p;
          if (!_this.editor.selection.rangeAtEndOf($node)) {
            return;
          }
          $p = $('<p/>').append(_this.editor.util.phBr).insertAfter($node);
          _this.editor.selection.setRangeAtStartOf($p);
          return true;
        };
      })(this);
      this.editor.inputManager.addKeystrokeHandler('13', 'h1', titleEnterHandler);
      this.editor.inputManager.addKeystrokeHandler('13', 'h2', titleEnterHandler);
      this.editor.inputManager.addKeystrokeHandler('13', 'h3', titleEnterHandler);
      this.editor.inputManager.addKeystrokeHandler('13', 'h4', titleEnterHandler);
      this.editor.inputManager.addKeystrokeHandler('13', 'h5', titleEnterHandler);
      this.editor.inputManager.addKeystrokeHandler('13', 'h6', titleEnterHandler);
    }
    this.editor.inputManager.addKeystrokeHandler('8', '*', (function(_this) {
      return function(e) {
        var $prevBlockEl, $rootBlock;
        $rootBlock = _this.editor.util.furthestBlockEl();
        $prevBlockEl = $rootBlock.prev();
        if ($prevBlockEl.is('hr') && _this.editor.selection.rangeAtStartOf($rootBlock)) {
          _this.editor.selection.save();
          $prevBlockEl.remove();
          _this.editor.selection.restore();
          return true;
        }
      };
    })(this));
    this.editor.inputManager.addKeystrokeHandler('9', '*', (function(_this) {
      return function(e) {
        var codeButton;
        codeButton = _this.editor.toolbar.findButton('code');
        if (!(_this.editor.opts.tabIndent || (codeButton && codeButton.active))) {
          return;
        }
        if (e.shiftKey) {
          _this.editor.util.outdent();
        } else {
          _this.editor.util.indent();
        }
        return true;
      };
    })(this));
    this.editor.inputManager.addKeystrokeHandler('13', 'li', (function(_this) {
      return function(e, $node) {
        var $cloneNode, listEl, newBlockEl, newListEl;
        $cloneNode = $node.clone();
        $cloneNode.find('ul, ol').remove();
        if (!(_this.editor.util.isEmptyNode($cloneNode) && $node.is(_this.editor.util.closestBlockEl()))) {
          return;
        }
        listEl = $node.parent();
        if ($node.next('li').length > 0) {
          if (!_this.editor.util.isEmptyNode($node)) {
            return;
          }
          if (listEl.parent('li').length > 0) {
            newBlockEl = $('<li/>').append(_this.editor.util.phBr).insertAfter(listEl.parent('li'));
            newListEl = $('<' + listEl[0].tagName + '/>').append($node.nextAll('li'));
            newBlockEl.append(newListEl);
          } else {
            newBlockEl = $('<p/>').append(_this.editor.util.phBr).insertAfter(listEl);
            newListEl = $('<' + listEl[0].tagName + '/>').append($node.nextAll('li'));
            newBlockEl.after(newListEl);
          }
        } else {
          if (listEl.parent('li').length > 0) {
            newBlockEl = $('<li/>').insertAfter(listEl.parent('li'));
            if ($node.contents().length > 0) {
              newBlockEl.append($node.contents());
            } else {
              newBlockEl.append(_this.editor.util.phBr);
            }
          } else {
            newBlockEl = $('<p/>').append(_this.editor.util.phBr).insertAfter(listEl);
            if ($node.children('ul, ol').length > 0) {
              newBlockEl.after($node.children('ul, ol'));
            }
          }
        }
        if ($node.prev('li').length) {
          $node.remove();
        } else {
          listEl.remove();
        }
        _this.editor.selection.setRangeAtStartOf(newBlockEl);
        return true;
      };
    })(this));
    this.editor.inputManager.addKeystrokeHandler('13', 'pre', (function(_this) {
      return function(e, $node) {
        var $p, breakNode, range;
        e.preventDefault();
        if (e.shiftKey) {
          $p = $('<p/>').append(_this.editor.util.phBr).insertAfter($node);
          _this.editor.selection.setRangeAtStartOf($p);
          return true;
        }
        range = _this.editor.selection.getRange();
        breakNode = null;
        range.deleteContents();
        if (!_this.editor.util.browser.msie && _this.editor.selection.rangeAtEndOf($node)) {
          breakNode = document.createTextNode('\n\n');
          range.insertNode(breakNode);
          range.setEnd(breakNode, 1);
        } else {
          breakNode = document.createTextNode('\n');
          range.insertNode(breakNode);
          range.setStartAfter(breakNode);
        }
        range.collapse(false);
        _this.editor.selection.selectRange(range);
        return true;
      };
    })(this));
    this.editor.inputManager.addKeystrokeHandler('13', 'blockquote', (function(_this) {
      return function(e, $node) {
        var $closestBlock, range;
        $closestBlock = _this.editor.util.closestBlockEl();
        if (!($closestBlock.is('p') && !$closestBlock.next().length && _this.editor.util.isEmptyNode($closestBlock))) {
          return;
        }
        $node.after($closestBlock);
        range = document.createRange();
        _this.editor.selection.setRangeAtStartOf($closestBlock, range);
        return true;
      };
    })(this));
    this.editor.inputManager.addKeystrokeHandler('8', 'li', (function(_this) {
      return function(e, $node) {
        var $br, $childList, $newLi, $prevChildList, $prevNode, $textNode, range, text;
        $childList = $node.children('ul, ol');
        $prevNode = $node.prev('li');
        if (!($childList.length > 0 && $prevNode.length > 0)) {
          return false;
        }
        text = '';
        $textNode = null;
        $node.contents().each(function(i, n) {
          if (n.nodeType === 1 && /UL|OL/.test(n.nodeName)) {
            return false;
          }
          if (n.nodeType === 1 && /BR/.test(n.nodeName)) {
            return;
          }
          if (n.nodeType === 3 && n.nodeValue) {
            text += n.nodeValue;
          } else if (n.nodeType === 1) {
            text += $(n).text();
          }
          return $textNode = $(n);
        });
        if ($textNode && text.length === 1 && _this.editor.util.browser.firefox && !$textNode.next('br').length) {
          $br = $(_this.editor.util.phBr).insertAfter($textNode);
          $textNode.remove();
          _this.editor.selection.setRangeBefore($br);
          return true;
        } else if (text.length > 0) {
          return false;
        }
        range = document.createRange();
        $prevChildList = $prevNode.children('ul, ol');
        if ($prevChildList.length > 0) {
          $newLi = $('<li/>').append(_this.editor.util.phBr).appendTo($prevChildList);
          $prevChildList.append($childList.children('li'));
          $node.remove();
          _this.editor.selection.setRangeAtEndOf($newLi, range);
        } else {
          _this.editor.selection.setRangeAtEndOf($prevNode, range);
          $prevNode.append($childList);
          $node.remove();
          _this.editor.selection.selectRange(range);
        }
        return true;
      };
    })(this));
    this.editor.inputManager.addKeystrokeHandler('8', 'pre', (function(_this) {
      return function(e, $node) {
        var $newNode, codeStr, range;
        if (!_this.editor.selection.rangeAtStartOf($node)) {
          return;
        }
        codeStr = $node.html().replace('\n', '<br/>');
        $newNode = $('<p/>').append(codeStr || _this.editor.util.phBr).insertAfter($node);
        $node.remove();
        range = document.createRange();
        _this.editor.selection.setRangeAtStartOf($newNode, range);
        return true;
      };
    })(this));
    return this.editor.inputManager.addKeystrokeHandler('8', 'blockquote', (function(_this) {
      return function(e, $node) {
        var $firstChild, range;
        if (!_this.editor.selection.rangeAtStartOf($node)) {
          return;
        }
        $firstChild = $node.children().first().unwrap();
        range = document.createRange();
        _this.editor.selection.setRangeAtStartOf($firstChild, range);
        return true;
      };
    })(this));
  };

  return Keystroke;

})(SimpleModule);

var UndoManager,
  __hasProp = {}.hasOwnProperty,
  __extends = function(child, parent) { for (var key in parent) { if (__hasProp.call(parent, key)) child[key] = parent[key]; } function ctor() { this.constructor = child; } ctor.prototype = parent.prototype; child.prototype = new ctor(); child.__super__ = parent.prototype; return child; };

UndoManager = (function(_super) {
  __extends(UndoManager, _super);

  function UndoManager() {
    return UndoManager.__super__.constructor.apply(this, arguments);
  }

  UndoManager.pluginName = 'UndoManager';

  UndoManager.prototype._index = -1;

  UndoManager.prototype._capacity = 50;

  UndoManager.prototype._timer = null;

  UndoManager.prototype._init = function() {
    var redoShortcut, undoShortcut;
    this.editor = this._module;
    this._stack = [];
    if (this.editor.util.os.mac) {
      undoShortcut = 'cmd+z';
      redoShortcut = 'shift+cmd+z';
    } else if (this.editor.util.os.win) {
      undoShortcut = 'ctrl+z';
      redoShortcut = 'ctrl+y';
    } else {
      undoShortcut = 'ctrl+z';
      redoShortcut = 'shift+ctrl+z';
    }
    this.editor.inputManager.addShortcut(undoShortcut, (function(_this) {
      return function(e) {
        e.preventDefault();
        _this.undo();
        return false;
      };
    })(this));
    this.editor.inputManager.addShortcut(redoShortcut, (function(_this) {
      return function(e) {
        e.preventDefault();
        _this.redo();
        return false;
      };
    })(this));
    return this.editor.on('valuechanged', (function(_this) {
      return function(e, src) {
        if (src === 'undo') {
          return;
        }
        if (_this._timer) {
          clearTimeout(_this._timer);
          _this._timer = null;
        }
        return _this._timer = setTimeout(function() {
          _this._pushUndoState();
          return _this._timer = null;
        }, 200);
      };
    })(this));
  };

  UndoManager.prototype._pushUndoState = function() {
    var currentState, html;
    if (this.editor.triggerHandler('pushundostate') === false) {
      return;
    }
    currentState = this.currentState();
    html = this.editor.body.html();
    if (currentState && currentState.html === html) {
      return;
    }
    this._index += 1;
    this._stack.length = this._index;
    this._stack.push({
      html: html,
      caret: this.caretPosition()
    });
    if (this._stack.length > this._capacity) {
      this._stack.shift();
      return this._index -= 1;
    }
  };

  UndoManager.prototype.currentState = function() {
    if (this._stack.length && this._index > -1) {
      return this._stack[this._index];
    } else {
      return null;
    }
  };

  UndoManager.prototype.undo = function() {
    var state;
    if (this._index < 1 || this._stack.length < 2) {
      return;
    }
    this.editor.hidePopover();
    this._index -= 1;
    state = this._stack[this._index];
    this.editor.body.html(state.html);
    this.caretPosition(state.caret);
    this.editor.body.find('.selected').removeClass('selected');
    this.editor.sync();
    return this.editor.trigger('valuechanged', ['undo']);
  };

  UndoManager.prototype.redo = function() {
    var state;
    if (this._index < 0 || this._stack.length < this._index + 2) {
      return;
    }
    this.editor.hidePopover();
    this._index += 1;
    state = this._stack[this._index];
    this.editor.body.html(state.html);
    this.caretPosition(state.caret);
    this.editor.body.find('.selected').removeClass('selected');
    this.editor.sync();
    return this.editor.trigger('valuechanged', ['undo']);
  };

  UndoManager.prototype.update = function() {
    var currentState, html;
    if (this._timer) {
      return;
    }
    currentState = this.currentState();
    if (!currentState) {
      return;
    }
    html = this.editor.body.html();
    currentState.html = html;
    return currentState.caret = this.caretPosition();
  };

  UndoManager.prototype._getNodeOffset = function(node, index) {
    var $parent, merging, offset;
    if (index) {
      $parent = $(node);
    } else {
      $parent = $(node).parent();
    }
    offset = 0;
    merging = false;
    $parent.contents().each((function(_this) {
      return function(i, child) {
        if (index === i || node === child) {
          return false;
        }
        if (child.nodeType === 3) {
          if (!merging) {
            offset += 1;
            merging = true;
          }
        } else {
          offset += 1;
          merging = false;
        }
        return null;
      };
    })(this));
    return offset;
  };

  UndoManager.prototype._getNodePosition = function(node, offset) {
    var position, prevNode;
    if (node.nodeType === 3) {
      prevNode = node.previousSibling;
      while (prevNode && prevNode.nodeType === 3) {
        node = prevNode;
        offset += this.editor.util.getNodeLength(prevNode);
        prevNode = prevNode.previousSibling;
      }
    } else {
      offset = this._getNodeOffset(node, offset);
    }
    position = [];
    position.unshift(offset);
    this.editor.util.traverseUp((function(_this) {
      return function(n) {
        return position.unshift(_this._getNodeOffset(n));
      };
    })(this), node);
    return position;
  };

  UndoManager.prototype._getNodeByPosition = function(position) {
    var child, childNodes, i, node, offset, _i, _len, _ref;
    node = this.editor.body[0];
    _ref = position.slice(0, position.length - 1);
    for (i = _i = 0, _len = _ref.length; _i < _len; i = ++_i) {
      offset = _ref[i];
      childNodes = node.childNodes;
      if (offset > childNodes.length - 1) {
        if (i === position.length - 2 && $(node).is('pre')) {
          child = document.createTextNode('');
          node.appendChild(child);
          childNodes = node.childNodes;
        } else {
          node = null;
          break;
        }
      }
      node = childNodes[offset];
    }
    return node;
  };

  UndoManager.prototype.caretPosition = function(caret) {
    var endContainer, endOffset, range, startContainer, startOffset;
    if (!caret) {
      range = this.editor.selection.getRange();
      if (!(this.editor.inputManager.focused && (range != null))) {
        return {};
      }
      caret = {
        start: [],
        end: null,
        collapsed: true
      };
      caret.start = this._getNodePosition(range.startContainer, range.startOffset);
      if (!range.collapsed) {
        caret.end = this._getNodePosition(range.endContainer, range.endOffset);
        caret.collapsed = false;
      }
      return caret;
    } else {
      if (!this.editor.inputManager.focused) {
        this.editor.body.focus();
      }
      if (!caret.start) {
        this.editor.body.blur();
        return;
      }
      startContainer = this._getNodeByPosition(caret.start);
      startOffset = caret.start[caret.start.length - 1];
      if (caret.collapsed) {
        endContainer = startContainer;
        endOffset = startOffset;
      } else {
        endContainer = this._getNodeByPosition(caret.end);
        endOffset = caret.start[caret.start.length - 1];
      }
      if (!startContainer || !endContainer) {
        throw new Error('simditor: invalid caret state');
        return;
      }
      range = document.createRange();
      range.setStart(startContainer, startOffset);
      range.setEnd(endContainer, endOffset);
      return this.editor.selection.selectRange(range);
    }
  };

  return UndoManager;

})(SimpleModule);

var Util,
  __hasProp = {}.hasOwnProperty,
  __extends = function(child, parent) { for (var key in parent) { if (__hasProp.call(parent, key)) child[key] = parent[key]; } function ctor() { this.constructor = child; } ctor.prototype = parent.prototype; child.prototype = new ctor(); child.__super__ = parent.prototype; return child; };

Util = (function(_super) {
  __extends(Util, _super);

  function Util() {
    return Util.__super__.constructor.apply(this, arguments);
  }

  Util.pluginName = 'Util';

  Util.prototype._init = function() {
    this.editor = this._module;
    if (this.browser.msie && this.browser.version < 11) {
      return this.phBr = '';
    }
  };

  Util.prototype.phBr = '<br/>';

  Util.prototype.os = (function() {
    var os;
    os = {};
    if (/Mac/.test(navigator.appVersion)) {
      os.mac = true;
    } else if (/Linux/.test(navigator.appVersion)) {
      os.linux = true;
    } else if (/Win/.test(navigator.appVersion)) {
      os.win = true;
    } else if (/X11/.test(navigator.appVersion)) {
      os.unix = true;
    }
    if (/Mobi/.test(navigator.appVersion)) {
      os.mobile = true;
    }
    return os;
  })();

  Util.prototype.browser = (function() {
    var chrome, firefox, ie, safari, ua, _ref, _ref1, _ref2, _ref3;
    ua = navigator.userAgent;
    ie = /(msie|trident)/i.test(ua);
    chrome = /chrome|crios/i.test(ua);
    safari = /safari/i.test(ua) && !chrome;
    firefox = /firefox/i.test(ua);
    if (ie) {
      return {
        msie: true,
        version: ((_ref = ua.match(/(msie |rv:)(\d+(\.\d+)?)/i)) != null ? _ref[2] : void 0) * 1
      };
    } else if (chrome) {
      return {
        webkit: true,
        chrome: true,
        version: ((_ref1 = ua.match(/(?:chrome|crios)\/(\d+(\.\d+)?)/i)) != null ? _ref1[1] : void 0) * 1
      };
    } else if (safari) {
      return {
        webkit: true,
        safari: true,
        version: ((_ref2 = ua.match(/version\/(\d+(\.\d+)?)/i)) != null ? _ref2[1] : void 0) * 1
      };
    } else if (firefox) {
      return {
        mozilla: true,
        firefox: true,
        version: ((_ref3 = ua.match(/firefox\/(\d+(\.\d+)?)/i)) != null ? _ref3[1] : void 0) * 1
      };
    } else {
      return {};
    }
  })();

  Util.prototype.supportSelectionChange = (function() {
    var e, onselectionchange;
    onselectionchange = document.onselectionchange;
    if (onselectionchange !== void 0) {
      try {
        document.onselectionchange = 0;
        return document.onselectionchange === null;
      } catch (_error) {
        e = _error;
      } finally {
        document.onselectionchange = onselectionchange;
      }
    }
    return false;
  })();

  Util.prototype.reflow = function(el) {
    if (el == null) {
      el = document;
    }
    return $(el)[0].offsetHeight;
  };

  Util.prototype.metaKey = function(e) {
    var isMac;
    isMac = /Mac/.test(navigator.userAgent);
    if (isMac) {
      return e.metaKey;
    } else {
      return e.ctrlKey;
    }
  };

  Util.prototype.isEmptyNode = function(node) {
    var $node;
    $node = $(node);
    return $node.is(':empty') || (!$node.text() && !$node.find(':not(br, span, div)').length);
  };

  Util.prototype.isBlockNode = function(node) {
    node = $(node)[0];
    if (!node || node.nodeType === 3) {
      return false;
    }
    return /^(div|p|ul|ol|li|blockquote|hr|pre|h1|h2|h3|h4|table)$/.test(node.nodeName.toLowerCase());
  };

  Util.prototype.closestBlockEl = function(node) {
    var $node, blockEl, range;
    if (node == null) {
      range = this.editor.selection.getRange();
      node = range != null ? range.commonAncestorContainer : void 0;
    }
    $node = $(node);
    if (!$node.length) {
      return null;
    }
    blockEl = $node.parentsUntil(this.editor.body).addBack();
    blockEl = blockEl.filter((function(_this) {
      return function(i) {
        return _this.isBlockNode(blockEl.eq(i));
      };
    })(this));
    if (blockEl.length) {
      return blockEl.last();
    } else {
      return null;
    }
  };

  Util.prototype.furthestNode = function(node, filter) {
    var $node, blockEl, range;
    if (node == null) {
      range = this.editor.selection.getRange();
      node = range != null ? range.commonAncestorContainer : void 0;
    }
    $node = $(node);
    if (!$node.length) {
      return null;
    }
    blockEl = $node.parentsUntil(this.editor.body).addBack();
    blockEl = blockEl.filter((function(_this) {
      return function(i) {
        var $n;
        $n = blockEl.eq(i);
        if ($.isFunction(filter)) {
          return filter($n);
        } else {
          return $n.is(filter);
        }
      };
    })(this));
    if (blockEl.length) {
      return blockEl.first();
    } else {
      return null;
    }
  };

  Util.prototype.furthestBlockEl = function(node) {
    return this.furthestNode(node, this.isBlockNode);
  };

  Util.prototype.getNodeLength = function(node) {
    switch (node.nodeType) {
      case 7:
      case 10:
        return 0;
      case 3:
      case 8:
        return node.length;
      default:
        return node.childNodes.length;
    }
  };

  Util.prototype.traverseUp = function(callback, node) {
    var n, nodes, range, result, _i, _len, _results;
    if (node == null) {
      range = this.editor.selection.getRange();
      node = range != null ? range.commonAncestorContainer : void 0;
    }
    if ((node == null) || !$.contains(this.editor.body[0], node)) {
      return false;
    }
    nodes = $(node).parentsUntil(this.editor.body).get();
    nodes.unshift(node);
    _results = [];
    for (_i = 0, _len = nodes.length; _i < _len; _i++) {
      n = nodes[_i];
      result = callback(n);
      if (result === false) {
        break;
      } else {
        _results.push(void 0);
      }
    }
    return _results;
  };

  Util.prototype.indent = function() {
    var $blockEl, $childList, $nextTd, $parentLi, $td, indentLevel, range, spaceNode, tagName, _ref;
    $blockEl = this.editor.util.closestBlockEl();
    if (!($blockEl && $blockEl.length > 0)) {
      return false;
    }
    if ($blockEl.is('pre')) {
      spaceNode = document.createTextNode('\u00A0\u00A0');
      this.editor.selection.insertNode(spaceNode);
    } else if ($blockEl.is('li')) {
      $parentLi = $blockEl.prev('li');
      if ($parentLi.length < 1) {
        return false;
      }
      this.editor.selection.save();
      tagName = $blockEl.parent()[0].tagName;
      $childList = $parentLi.children('ul, ol');
      if ($childList.length > 0) {
        $childList.append($blockEl);
      } else {
        $('<' + tagName + '/>').append($blockEl).appendTo($parentLi);
      }
      this.editor.selection.restore();
    } else if ($blockEl.is('p, h1, h2, h3, h4')) {
      indentLevel = (_ref = $blockEl.attr('data-indent')) != null ? _ref : 0;
      indentLevel = indentLevel * 1 + 1;
      if (indentLevel > 10) {
        indentLevel = 10;
      }
      $blockEl.attr('data-indent', indentLevel);
    } else if ($blockEl.is('table')) {
      range = this.editor.selection.getRange();
      $td = $(range.commonAncestorContainer).closest('td');
      $nextTd = $td.next('td');
      if (!($nextTd.length > 0)) {
        $nextTd = $td.parent('tr').next('tr').find('td:first');
      }
      if (!($td.length > 0 && $nextTd.length > 0)) {
        return false;
      }
      this.editor.selection.setRangeAtEndOf($nextTd);
    } else {
      spaceNode = document.createTextNode('\u00A0\u00A0\u00A0\u00A0');
      this.editor.selection.insertNode(spaceNode);
    }
    this.editor.trigger('valuechanged');
    return true;
  };

  Util.prototype.outdent = function() {
    var $blockEl, $parent, $parentLi, $prevTd, $td, button, indentLevel, range, _ref;
    $blockEl = this.editor.util.closestBlockEl();
    if (!($blockEl && $blockEl.length > 0)) {
      return false;
    }
    if ($blockEl.is('pre')) {
      return false;
    } else if ($blockEl.is('li')) {
      $parent = $blockEl.parent();
      $parentLi = $parent.parent('li');
      if ($parentLi.length < 1) {
        button = this.editor.toolbar.findButton($parent[0].tagName.toLowerCase());
        if (button != null) {
          button.command();
        }
        return false;
      }
      this.editor.selection.save();
      if ($blockEl.next('li').length > 0) {
        $('<' + $parent[0].tagName + '/>').append($blockEl.nextAll('li')).appendTo($blockEl);
      }
      $blockEl.insertAfter($parentLi);
      if ($parent.children('li').length < 1) {
        $parent.remove();
      }
      this.editor.selection.restore();
    } else if ($blockEl.is('p, h1, h2, h3, h4')) {
      indentLevel = (_ref = $blockEl.attr('data-indent')) != null ? _ref : 0;
      indentLevel = indentLevel * 1 - 1;
      if (indentLevel < 0) {
        indentLevel = 0;
      }
      $blockEl.attr('data-indent', indentLevel);
    } else if ($blockEl.is('table')) {
      range = this.editor.selection.getRange();
      $td = $(range.commonAncestorContainer).closest('td');
      $prevTd = $td.prev('td');
      if (!($prevTd.length > 0)) {
        $prevTd = $td.parent('tr').prev('tr').find('td:last');
      }
      if (!($td.length > 0 && $prevTd.length > 0)) {
        return false;
      }
      this.editor.selection.setRangeAtEndOf($prevTd);
    } else {
      return false;
    }
    this.editor.trigger('valuechanged');
    return true;
  };

  Util.prototype.dataURLtoBlob = function(dataURL) {
    var BlobBuilder, arrayBuffer, bb, byteString, hasArrayBufferViewSupport, hasBlobConstructor, i, intArray, mimeString, _i, _ref;
    hasBlobConstructor = window.Blob && (function() {
      var e;
      try {
        return Boolean(new Blob());
      } catch (_error) {
        e = _error;
        return false;
      }
    })();
    hasArrayBufferViewSupport = hasBlobConstructor && window.Uint8Array && (function() {
      var e;
      try {
        return new Blob([new Uint8Array(100)]).size === 100;
      } catch (_error) {
        e = _error;
        return false;
      }
    })();
    BlobBuilder = window.BlobBuilder || window.WebKitBlobBuilder || window.MozBlobBuilder || window.MSBlobBuilder;
    if (!((hasBlobConstructor || BlobBuilder) && window.atob && window.ArrayBuffer && window.Uint8Array)) {
      return false;
    }
    if (dataURL.split(',')[0].indexOf('base64') >= 0) {
      byteString = atob(dataURL.split(',')[1]);
    } else {
      byteString = decodeURIComponent(dataURL.split(',')[1]);
    }
    arrayBuffer = new ArrayBuffer(byteString.length);
    intArray = new Uint8Array(arrayBuffer);
    for (i = _i = 0, _ref = byteString.length; 0 <= _ref ? _i <= _ref : _i >= _ref; i = 0 <= _ref ? ++_i : --_i) {
      intArray[i] = byteString.charCodeAt(i);
    }
    mimeString = dataURL.split(',')[0].split(':')[1].split(';')[0];
    if (hasBlobConstructor) {
      return new Blob([hasArrayBufferViewSupport ? intArray : arrayBuffer], {
        type: mimeString
      });
    }
    bb = new BlobBuilder();
    bb.append(arrayBuffer);
    return bb.getBlob(mimeString);
  };

  return Util;

})(SimpleModule);

var Toolbar,
  __hasProp = {}.hasOwnProperty,
  __extends = function(child, parent) { for (var key in parent) { if (__hasProp.call(parent, key)) child[key] = parent[key]; } function ctor() { this.constructor = child; } ctor.prototype = parent.prototype; child.prototype = new ctor(); child.__super__ = parent.prototype; return child; };

Toolbar = (function(_super) {
  __extends(Toolbar, _super);

  function Toolbar() {
    return Toolbar.__super__.constructor.apply(this, arguments);
  }

  Toolbar.pluginName = 'Toolbar';

  Toolbar.prototype.opts = {
    toolbar: true,
    toolbarFloat: true,
    toolbarHidden: false,
    toolbarFloatOffset: 0
  };

  Toolbar.prototype._tpl = {
    wrapper: '<div class="simditor-toolbar"><ul></ul></div>',
    separator: '<li><span class="separator"></span></li>'
  };

  Toolbar.prototype._init = function() {
    var toolbarHeight;
    this.editor = this._module;
    if (!this.opts.toolbar) {
      return;
    }
    if (!$.isArray(this.opts.toolbar)) {
      this.opts.toolbar = ['bold', 'italic', 'underline', 'strikethrough', '|', 'ol', 'ul', 'blockquote', 'code', '|', 'link', 'image', '|', 'indent', 'outdent'];
    }
    this._render();
    this.list.on('click', (function(_this) {
      return function(e) {
        return false;
      };
    })(this));
    this.wrapper.on('mousedown', (function(_this) {
      return function(e) {
        return _this.list.find('.menu-on').removeClass('.menu-on');
      };
    })(this));
    $(document).on('mousedown.simditor' + this.editor.id, (function(_this) {
      return function(e) {
        return _this.list.find('.menu-on').removeClass('.menu-on');
      };
    })(this));
    if (!this.opts.toolbarHidden && this.opts.toolbarFloat) {
      this.wrapper.width(this.wrapper.outerWidth());
      this.wrapper.css('top', this.opts.toolbarFloatOffset);
      toolbarHeight = this.wrapper.outerHeight();
      if (!this.editor.util.os.mobile) {
        $(window).on('resize.simditor-' + this.editor.id, (function(_this) {
          return function(e) {
            _this.wrapper.css('position', 'static');
            _this.editor.util.reflow(_this.wrapper);
            _this.wrapper.css('left', _this.wrapper.offset().left);
            return _this.wrapper.css('position', '');
          };
        })(this)).resize();
      }
      $(window).on('scroll.simditor-' + this.editor.id, (function(_this) {
        return function(e) {
          var bottomEdge, scrollTop, topEdge;
          topEdge = _this.editor.wrapper.offset().top;
          bottomEdge = topEdge + _this.editor.wrapper.outerHeight() - 80;
          scrollTop = $(document).scrollTop() + _this.opts.toolbarFloatOffset;
          if (scrollTop <= topEdge || scrollTop >= bottomEdge) {
            _this.editor.wrapper.removeClass('toolbar-floating').css('padding-top', '');
            if (_this.editor.util.os.mobile) {
              return _this.wrapper.css('top', _this.opts.toolbarFloatOffset);
            }
          } else {
            _this.editor.wrapper.addClass('toolbar-floating').css('padding-top', toolbarHeight);
            if (_this.editor.util.os.mobile) {
              return _this.wrapper.css('top', scrollTop - topEdge + _this.opts.toolbarFloatOffset);
            }
          }
        };
      })(this));
    }
    this.editor.on('selectionchanged', (function(_this) {
      return function() {
        return _this.toolbarStatus();
      };
    })(this));
    this.editor.on('destroy', (function(_this) {
      return function() {
        return _this.buttons.length = 0;
      };
    })(this));
    return $(document).on('mousedown.simditor-' + this.editor.id, (function(_this) {
      return function(e) {
        return _this.list.find('li.menu-on').removeClass('menu-on');
      };
    })(this));
  };

  Toolbar.prototype._render = function() {
    var name, _i, _len, _ref;
    this.buttons = [];
    this.wrapper = $(this._tpl.wrapper).prependTo(this.editor.wrapper);
    this.list = this.wrapper.find('ul');
    _ref = this.opts.toolbar;
    for (_i = 0, _len = _ref.length; _i < _len; _i++) {
      name = _ref[_i];
      if (name === '|') {
        $(this._tpl.separator).appendTo(this.list);
        continue;
      }
      if (!this.constructor.buttons[name]) {
        throw new Error('simditor: invalid toolbar button "' + name + '"');
        continue;
      }
      this.buttons.push(new this.constructor.buttons[name]({
        editor: this.editor
      }));
    }
    if (this.opts.toolbarHidden) {
      return this.wrapper.hide();
    } else {
      return this.editor.placeholderEl.css('top', this.wrapper.outerHeight());
    }
  };

  Toolbar.prototype.toolbarStatus = function(name) {
    var buttons;
    if (!this.editor.inputManager.focused) {
      return;
    }
    buttons = this.buttons.slice(0);
    return this.editor.util.traverseUp((function(_this) {
      return function(node) {
        var button, i, removeButtons, _i, _j, _len, _len1;
        removeButtons = [];
        for (i = _i = 0, _len = buttons.length; _i < _len; i = ++_i) {
          button = buttons[i];
          if ((name != null) && button.name !== name) {
            continue;
          }
          if (!button.status || button.status($(node)) === true) {
            removeButtons.push(button);
          }
        }
        for (_j = 0, _len1 = removeButtons.length; _j < _len1; _j++) {
          button = removeButtons[_j];
          i = $.inArray(button, buttons);
          buttons.splice(i, 1);
        }
        if (buttons.length === 0) {
          return false;
        }
      };
    })(this));
  };

  Toolbar.prototype.findButton = function(name) {
    var button;
    button = this.list.find('.toolbar-item-' + name).data('button');
    return button != null ? button : null;
  };

  Toolbar.addButton = function(btn) {
    return this.buttons[btn.prototype.name] = btn;
  };

  Toolbar.buttons = {};

  return Toolbar;

})(SimpleModule);

var Simditor,
  __hasProp = {}.hasOwnProperty,
  __extends = function(child, parent) { for (var key in parent) { if (__hasProp.call(parent, key)) child[key] = parent[key]; } function ctor() { this.constructor = child; } ctor.prototype = parent.prototype; child.prototype = new ctor(); child.__super__ = parent.prototype; return child; };

Simditor = (function(_super) {
  __extends(Simditor, _super);

  function Simditor() {
    return Simditor.__super__.constructor.apply(this, arguments);
  }

  Simditor.connect(Util);

  Simditor.connect(InputManager);

  Simditor.connect(UndoManager);

  Simditor.connect(Keystroke);

  Simditor.connect(Formatter);

  Simditor.connect(Selection);

  Simditor.connect(Toolbar);

  Simditor.count = 0;

  Simditor.prototype.opts = {
    textarea: null,
    placeholder: '',
    defaultImage: 'images/image.png',
    params: {},
    upload: false,
    tabIndent: true
  };

  Simditor.prototype._init = function() {
    var e, editor, form, uploadOpts;
    this.textarea = $(this.opts.textarea);
    this.opts.placeholder = this.opts.placeholder || this.textarea.attr('placeholder');
    if (!this.textarea.length) {
      throw new Error('simditor: param textarea is required.');
      return;
    }
    editor = this.textarea.data('simditor');
    if (editor != null) {
      editor.destroy();
    }
    this.id = ++Simditor.count;
    this._render();
    if (this.opts.upload && simpleUploader) {
      uploadOpts = typeof this.opts.upload === 'object' ? this.opts.upload : {};
      this.uploader = simpleUploader(uploadOpts);
    }
    form = this.textarea.closest('form');
    if (form.length) {
      form.on('submit.simditor-' + this.id, (function(_this) {
        return function() {
          return _this.sync();
        };
      })(this));
      form.on('reset.simditor-' + this.id, (function(_this) {
        return function() {
          return _this.setValue('');
        };
      })(this));
    }
    this.on('initialized', (function(_this) {
      return function() {
        if (_this.opts.placeholder) {
          _this.on('valuechanged', function() {
            return _this._placeholder();
          });
        }
        return _this.setValue(_this.textarea.val().trim() || '');
      };
    })(this));
    if (this.util.browser.mozilla) {
      this.util.reflow();
      try {
        document.execCommand("enableObjectResizing", false, false);
        return document.execCommand("enableInlineTableEditing", false, false);
      } catch (_error) {
        e = _error;
      }
    }
  };

  Simditor.prototype._tpl = "<div class=\"simditor\">\n  <div class=\"simditor-wrapper\">\n    <div class=\"simditor-placeholder\"></div>\n    <div class=\"simditor-body\" contenteditable=\"true\">\n    </div>\n  </div>\n</div>";

  Simditor.prototype._render = function() {
    var key, val, _ref, _results;
    this.el = $(this._tpl).insertBefore(this.textarea);
    this.wrapper = this.el.find('.simditor-wrapper');
    this.body = this.wrapper.find('.simditor-body');
    this.placeholderEl = this.wrapper.find('.simditor-placeholder').append(this.opts.placeholder);
    this.el.append(this.textarea).data('simditor', this);
    this.textarea.data('simditor', this).hide().blur();
    this.body.attr('tabindex', this.textarea.attr('tabindex'));
    if (this.util.os.mac) {
      this.el.addClass('simditor-mac');
    } else if (this.util.os.linux) {
      this.el.addClass('simditor-linux');
    }
    if (this.util.os.mobile) {
      this.el.addClass('simditor-mobile');
    }
    if (this.opts.params) {
      _ref = this.opts.params;
      _results = [];
      for (key in _ref) {
        val = _ref[key];
        _results.push($('<input/>', {
          type: 'hidden',
          name: key,
          value: val
        }).insertAfter(this.textarea));
      }
      return _results;
    }
  };

  Simditor.prototype._placeholder = function() {
    var children, _ref;
    children = this.body.children();
    if (children.length === 0 || (children.length === 1 && this.util.isEmptyNode(children) && ((_ref = children.data('indent')) != null ? _ref : 0) < 1)) {
      return this.placeholderEl.show();
    } else {
      return this.placeholderEl.hide();
    }
  };

  Simditor.prototype.setValue = function(val) {
    this.hidePopover();
    this.textarea.val(val);
    this.body.html(val);
    this.formatter.format();
    this.formatter.decorate();
    this.util.reflow(this.body);
    this.inputManager.lastCaretPosition = null;
    return this.trigger('valuechanged');
  };

  Simditor.prototype.getValue = function() {
    return this.sync();
  };

  Simditor.prototype.sync = function() {
    var children, cloneBody, emptyP, firstP, lastP, val;
    cloneBody = this.body.clone();
    this.formatter.undecorate(cloneBody);
    this.formatter.format(cloneBody);
    this.formatter.autolink(cloneBody);
    children = cloneBody.children();
    lastP = children.last('p');
    firstP = children.first('p');
    while (lastP.is('p') && this.util.isEmptyNode(lastP)) {
      emptyP = lastP;
      lastP = lastP.prev('p');
      emptyP.remove();
    }
    while (firstP.is('p') && this.util.isEmptyNode(firstP)) {
      emptyP = firstP;
      firstP = lastP.next('p');
      emptyP.remove();
    }
    cloneBody.find('img.uploading').remove();
    val = $.trim(cloneBody.html());
    this.textarea.val(val);
    return val;
  };

  Simditor.prototype.focus = function() {
    var $blockEl, range;
    if (this.inputManager.lastCaretPosition) {
      return this.undoManager.caretPosition(this.inputManager.lastCaretPosition);
    } else {
      $blockEl = this.body.find('p, li, pre, h1, h2, h3, h4, td').first();
      if (!($blockEl.length > 0)) {
        return;
      }
      range = document.createRange();
      this.selection.setRangeAtStartOf($blockEl, range);
      return this.body.focus();
    }
  };

  Simditor.prototype.blur = function() {
    return this.body.blur();
  };

  Simditor.prototype.hidePopover = function() {
    return this.el.find('.simditor-popover').each((function(_this) {
      return function(i, popover) {
        popover = $(popover).data('popover');
        if (popover.active) {
          return popover.hide();
        }
      };
    })(this));
  };

  Simditor.prototype.destroy = function() {
    this.triggerHandler('destroy');
    this.textarea.closest('form').off('.simditor .simditor-' + this.id);
    this.selection.clear();
    this.inputManager.focused = false;
    this.textarea.insertBefore(this.el).hide().val('').removeData('simditor');
    this.el.remove();
    $(document).off('.simditor-' + this.id);
    $(window).off('.simditor-' + this.id);
    return this.off();
  };

  return Simditor;

})(SimpleModule);

Simditor.i18n = {
  'zh-CN': {
    'blockquote': '引用',
    'bold': '加粗文字',
    'code': '插入代码',
    'color': '文字颜色',
    'hr': '分隔线',
    'image': '插入图片',
    'localImage': '本地图片',
    'externalImage': '外链图片',
    'uploadImage': '上传图片',
    'uploadFailed': '上传失败了',
    'uploadError': '上传出错了',
    'imageUrl': '图片地址',
    'imageSize': '图片尺寸',
    'restoreImageSize': '还原图片尺寸',
    'uploading': '正在上传',
    'indent': '向右缩进',
    'outdent': '向左缩进',
    'italic': '斜体文字',
    'link': '插入链接',
    'text': '文本',
    'linkText': '链接文字',
    'linkUrl': '地址',
    'removeLink': '移除链接',
    'ol': '有序列表',
    'ul': '无序列表',
    'strikethrough': '删除线文字',
    'table': '表格',
    'deleteRow': '删除行',
    'insertRowAbove': '在上面插入行',
    'insertRowBelow': '在下面插入行',
    'deleteColumn': '删除列',
    'insertColumnLeft': '在左边插入列',
    'insertColumnRight': '在右边插入列',
    'deleteTable': '删除表格',
    'title': '标题',
    'normalText': '普通文本',
    'underline': '下划线文字'
  }
};

var Button,
  __hasProp = {}.hasOwnProperty,
  __extends = function(child, parent) { for (var key in parent) { if (__hasProp.call(parent, key)) child[key] = parent[key]; } function ctor() { this.constructor = child; } ctor.prototype = parent.prototype; child.prototype = new ctor(); child.__super__ = parent.prototype; return child; },
  __slice = [].slice;

Button = (function(_super) {
  __extends(Button, _super);

  Button.prototype._tpl = {
    item: '<li><a tabindex="-1" unselectable="on" class="toolbar-item" href="javascript:;"><span></span></a></li>',
    menuWrapper: '<div class="toolbar-menu"></div>',
    menuItem: '<li><a tabindex="-1" unselectable="on" class="menu-item" href="javascript:;"><span></span></a></li>',
    separator: '<li><span class="separator"></span></li>'
  };

  Button.prototype.name = '';

  Button.prototype.icon = '';

  Button.prototype.title = '';

  Button.prototype.text = '';

  Button.prototype.htmlTag = '';

  Button.prototype.disableTag = '';

  Button.prototype.menu = false;

  Button.prototype.active = false;

  Button.prototype.disabled = false;

  Button.prototype.needFocus = true;

  Button.prototype.shortcut = null;

  function Button(opts) {
    this.editor = opts.editor;
    this.title = this._t(this.name);
    Button.__super__.constructor.call(this, opts);
  }

  Button.prototype._init = function() {
    var tag, _i, _len, _ref, _results;
    this.render();
    this.el.on('mousedown', (function(_this) {
      return function(e) {
        var exceed, param;
        e.preventDefault();
        if (_this.el.hasClass('disabled') || (_this.needFocus && !_this.editor.inputManager.focused)) {
          return false;
        }
        if (_this.menu) {
          _this.wrapper.toggleClass('menu-on').siblings('li').removeClass('menu-on');
          if (_this.wrapper.is('.menu-on')) {
            exceed = _this.menuWrapper.offset().left + _this.menuWrapper.outerWidth() + 5 - _this.editor.wrapper.offset().left - _this.editor.wrapper.outerWidth();
            if (exceed > 0) {
              _this.menuWrapper.css({
                'left': 'auto',
                'right': 0
              });
            }
            _this.trigger('menuexpand');
          }
          return false;
        }
        param = _this.el.data('param');
        _this.command(param);
        return false;
      };
    })(this));
    this.wrapper.on('click', 'a.menu-item', (function(_this) {
      return function(e) {
        var btn, param;
        e.preventDefault();
        btn = $(e.currentTarget);
        _this.wrapper.removeClass('menu-on');
        if (btn.hasClass('disabled') || (_this.needFocus && !_this.editor.inputManager.focused)) {
          return false;
        }
        _this.editor.toolbar.wrapper.removeClass('menu-on');
        param = btn.data('param');
        _this.command(param);
        return false;
      };
    })(this));
    this.wrapper.on('mousedown', 'a.menu-item', (function(_this) {
      return function(e) {
        return false;
      };
    })(this));
    this.editor.on('blur', (function(_this) {
      return function() {
        _this.setActive(false);
        return _this.setDisabled(false);
      };
    })(this));
    if (this.shortcut != null) {
      this.editor.inputManager.addShortcut(this.shortcut, (function(_this) {
        return function(e) {
          _this.el.mousedown();
          return false;
        };
      })(this));
    }
    _ref = this.htmlTag.split(',');
    _results = [];
    for (_i = 0, _len = _ref.length; _i < _len; _i++) {
      tag = _ref[_i];
      tag = $.trim(tag);
      if (tag && $.inArray(tag, this.editor.formatter._allowedTags) < 0) {
        _results.push(this.editor.formatter._allowedTags.push(tag));
      } else {
        _results.push(void 0);
      }
    }
    return _results;
  };

  Button.prototype.render = function() {
    this.wrapper = $(this._tpl.item).appendTo(this.editor.toolbar.list);
    this.el = this.wrapper.find('a.toolbar-item');
    this.el.attr('title', this.title).addClass('toolbar-item-' + this.name).data('button', this);
    this.el.find('span').addClass(this.icon ? 'fa fa-' + this.icon : '').text(this.text);
    if (!this.menu) {
      return;
    }
    this.menuWrapper = $(this._tpl.menuWrapper).appendTo(this.wrapper);
    this.menuWrapper.addClass('toolbar-menu-' + this.name);
    return this.renderMenu();
  };

  Button.prototype.renderMenu = function() {
    var $menuBtntnEl, $menuItemEl, menuItem, _i, _len, _ref, _ref1, _results;
    if (!$.isArray(this.menu)) {
      return;
    }
    this.menuEl = $('<ul/>').appendTo(this.menuWrapper);
    _ref = this.menu;
    _results = [];
    for (_i = 0, _len = _ref.length; _i < _len; _i++) {
      menuItem = _ref[_i];
      if (menuItem === '|') {
        $(this._tpl.separator).appendTo(this.menuEl);
        continue;
      }
      $menuItemEl = $(this._tpl.menuItem).appendTo(this.menuEl);
      _results.push($menuBtntnEl = $menuItemEl.find('a.menu-item').attr({
        'title': (_ref1 = menuItem.title) != null ? _ref1 : menuItem.text,
        'data-param': menuItem.param
      }).addClass('menu-item-' + menuItem.name).find('span').text(menuItem.text));
    }
    return _results;
  };

  Button.prototype.setActive = function(active) {
    if (active === this.active) {
      return;
    }
    this.active = active;
    this.el.toggleClass('active', this.active);
    return this.editor.toolbar.trigger('buttonstatus', [this]);
  };

  Button.prototype.setDisabled = function(disabled) {
    if (disabled === this.disabled) {
      return;
    }
    this.disabled = disabled;
    this.el.toggleClass('disabled', this.disabled);
    return this.editor.toolbar.trigger('buttonstatus', [this]);
  };

  Button.prototype.status = function($node) {
    if ($node != null) {
      this.setDisabled($node.is(this.disableTag));
    }
    if (this.disabled) {
      return true;
    }
    if ($node != null) {
      this.setActive($node.is(this.htmlTag));
    }
    return this.active;
  };

  Button.prototype.command = function(param) {};

  Button.prototype._t = function() {
    var args, result, _ref;
    args = 1 <= arguments.length ? __slice.call(arguments, 0) : [];
    result = Button.__super__._t.apply(this, args);
    if (!result) {
      result = (_ref = this.editor)._t.apply(_ref, args);
    }
    return result;
  };

  return Button;

})(SimpleModule);

Simditor.Button = Button;

var Popover,
  __hasProp = {}.hasOwnProperty,
  __extends = function(child, parent) { for (var key in parent) { if (__hasProp.call(parent, key)) child[key] = parent[key]; } function ctor() { this.constructor = child; } ctor.prototype = parent.prototype; child.prototype = new ctor(); child.__super__ = parent.prototype; return child; };

Popover = (function(_super) {
  __extends(Popover, _super);

  Popover.prototype.offset = {
    top: 4,
    left: 0
  };

  Popover.prototype.target = null;

  Popover.prototype.active = false;

  function Popover(opts) {
    this.button = opts.button;
    this.editor = opts.button.editor;
    Popover.__super__.constructor.call(this, opts);
  }

  Popover.prototype._init = function() {
    this.el = $('<div class="simditor-popover"></div>').appendTo(this.editor.el).data('popover', this);
    this.render();
    this.el.on('mouseenter', (function(_this) {
      return function(e) {
        return _this.el.addClass('hover');
      };
    })(this));
    return this.el.on('mouseleave', (function(_this) {
      return function(e) {
        return _this.el.removeClass('hover');
      };
    })(this));
  };

  Popover.prototype.render = function() {};

  Popover.prototype.show = function($target, position) {
    if (position == null) {
      position = 'bottom';
    }
    if ($target == null) {
      return;
    }
    this.el.siblings('.simditor-popover').each((function(_this) {
      return function(i, popover) {
        popover = $(popover).data('popover');
        if (popover.active) {
          return popover.hide();
        }
      };
    })(this));
    this.target = $target.addClass('selected');
    if (this.active) {
      this.refresh(position);
      return this.trigger('popovershow');
    } else {
      this.active = true;
      this.el.css({
        left: -9999
      }).show();
      return setTimeout((function(_this) {
        return function() {
          _this.refresh(position);
          return _this.trigger('popovershow');
        };
      })(this), 0);
    }
  };

  Popover.prototype.hide = function() {
    if (!this.active) {
      return;
    }
    if (this.target) {
      this.target.removeClass('selected');
    }
    this.target = null;
    this.active = false;
    this.el.hide();
    return this.trigger('popoverhide');
  };

  Popover.prototype.refresh = function(position) {
    var editorOffset, left, targetH, targetOffset, top;
    if (position == null) {
      position = 'bottom';
    }
    if (!this.active) {
      return;
    }
    editorOffset = this.editor.el.offset();
    targetOffset = this.target.offset();
    targetH = this.target.outerHeight();
    if (position === 'bottom') {
      top = targetOffset.top - editorOffset.top + targetH;
    } else if (position === 'top') {
      top = targetOffset.top - editorOffset.top - this.el.height();
    }
    left = Math.min(targetOffset.left - editorOffset.left, this.editor.wrapper.width() - this.el.outerWidth() - 10);
    return this.el.css({
      top: top + this.offset.top,
      left: left + this.offset.left
    });
  };

  Popover.prototype.destroy = function() {
    this.target = null;
    this.active = false;
    this.editor.off('.linkpopover');
    return this.el.remove();
  };

  return Popover;

})(SimpleModule);

Simditor.Popover = Popover;

var TitleButton,
  __hasProp = {}.hasOwnProperty,
  __extends = function(child, parent) { for (var key in parent) { if (__hasProp.call(parent, key)) child[key] = parent[key]; } function ctor() { this.constructor = child; } ctor.prototype = parent.prototype; child.prototype = new ctor(); child.__super__ = parent.prototype; return child; };

TitleButton = (function(_super) {
  __extends(TitleButton, _super);

  function TitleButton() {
    return TitleButton.__super__.constructor.apply(this, arguments);
  }

  TitleButton.prototype.name = 'title';

  TitleButton.prototype.htmlTag = 'h1, h2, h3, h4';

  TitleButton.prototype.disableTag = 'pre, table';

  TitleButton.prototype._init = function() {
    this.menu = [
      {
        name: 'normal',
        text: this._t('normalText'),
        param: 'p'
      }, '|', {
        name: 'h1',
        text: this._t('title') + ' 1',
        param: 'h1'
      }, {
        name: 'h2',
        text: this._t('title') + ' 2',
        param: 'h2'
      }, {
        name: 'h3',
        text: this._t('title') + ' 3',
        param: 'h3'
      }, {
        name: 'h4',
        text: this._t('title') + ' 4',
        param: 'h4'
      }, {
        name: 'h5',
        text: this._t('title') + ' 5',
        param: 'h5'
      }
    ];
    return TitleButton.__super__._init.call(this);
  };

  TitleButton.prototype.setActive = function(active, param) {
    TitleButton.__super__.setActive.call(this, active);
    this.el.removeClass('active-p active-h1 active-h2 active-h3');
    if (active) {
      return this.el.addClass('active active-' + param);
    }
  };

  TitleButton.prototype.status = function($node) {
    var param, _ref;
    if ($node != null) {
      this.setDisabled($node.is(this.disableTag));
    }
    if (this.disabled) {
      return true;
    }
    if ($node != null) {
      param = (_ref = $node[0].tagName) != null ? _ref.toLowerCase() : void 0;
      this.setActive($node.is(this.htmlTag), param);
    }
    return this.active;
  };

  TitleButton.prototype.command = function(param) {
    var $contents, $endBlock, $startBlock, endNode, node, range, results, startNode, _i, _len, _ref;
    range = this.editor.selection.getRange();
    startNode = range.startContainer;
    endNode = range.endContainer;
    $startBlock = this.editor.util.closestBlockEl(startNode);
    $endBlock = this.editor.util.closestBlockEl(endNode);
    this.editor.selection.save();
    range.setStartBefore($startBlock[0]);
    range.setEndAfter($endBlock[0]);
    $contents = $(range.extractContents());
    results = [];
    $contents.children().each((function(_this) {
      return function(i, el) {
        var c, converted, _i, _len, _results;
        converted = _this._convertEl(el, param);
        _results = [];
        for (_i = 0, _len = converted.length; _i < _len; _i++) {
          c = converted[_i];
          _results.push(results.push(c));
        }
        return _results;
      };
    })(this));
    _ref = results.reverse();
    for (_i = 0, _len = _ref.length; _i < _len; _i++) {
      node = _ref[_i];
      range.insertNode(node[0]);
    }
    this.editor.selection.restore();
    return this.editor.trigger('valuechanged');
  };

  TitleButton.prototype._convertEl = function(el, param) {
    var $block, $el, results;
    $el = $(el);
    results = [];
    if ($el.is(param)) {
      results.push($el);
    } else {
      $block = $('<' + param + '/>').append($el.contents());
      results.push($block);
    }
    return results;
  };

  return TitleButton;

})(Button);

Simditor.Toolbar.addButton(TitleButton);

var BoldButton,
  __hasProp = {}.hasOwnProperty,
  __extends = function(child, parent) { for (var key in parent) { if (__hasProp.call(parent, key)) child[key] = parent[key]; } function ctor() { this.constructor = child; } ctor.prototype = parent.prototype; child.prototype = new ctor(); child.__super__ = parent.prototype; return child; };

BoldButton = (function(_super) {
  __extends(BoldButton, _super);

  function BoldButton() {
    return BoldButton.__super__.constructor.apply(this, arguments);
  }

  BoldButton.prototype.name = 'bold';

  BoldButton.prototype.icon = 'bold';

  BoldButton.prototype.htmlTag = 'b, strong';

  BoldButton.prototype.disableTag = 'pre';

  BoldButton.prototype.shortcut = 'cmd+b';

  BoldButton.prototype._init = function() {
    if (this.editor.util.os.mac) {
      this.title = this.title + ' ( Cmd + b )';
    } else {
      this.title = this.title + ' ( Ctrl + b )';
      this.shortcut = 'ctrl+b';
    }
    return BoldButton.__super__._init.call(this);
  };

  BoldButton.prototype.status = function($node) {
    var active;
    if ($node != null) {
      this.setDisabled($node.is(this.disableTag));
    }
    if (this.disabled) {
      return true;
    }
    active = document.queryCommandState('bold') === true;
    this.setActive(active);
    return active;
  };

  BoldButton.prototype.command = function() {
    document.execCommand('bold');
    this.editor.trigger('valuechanged');
    return $(document).trigger('selectionchange');
  };

  return BoldButton;

})(Button);

Simditor.Toolbar.addButton(BoldButton);

var ItalicButton,
  __hasProp = {}.hasOwnProperty,
  __extends = function(child, parent) { for (var key in parent) { if (__hasProp.call(parent, key)) child[key] = parent[key]; } function ctor() { this.constructor = child; } ctor.prototype = parent.prototype; child.prototype = new ctor(); child.__super__ = parent.prototype; return child; };

ItalicButton = (function(_super) {
  __extends(ItalicButton, _super);

  function ItalicButton() {
    return ItalicButton.__super__.constructor.apply(this, arguments);
  }

  ItalicButton.prototype.name = 'italic';

  ItalicButton.prototype.icon = 'italic';

  ItalicButton.prototype.htmlTag = 'i';

  ItalicButton.prototype.disableTag = 'pre';

  ItalicButton.prototype.shortcut = 'cmd+i';

  ItalicButton.prototype._init = function() {
    if (this.editor.util.os.mac) {
      this.title = this.title + ' ( Cmd + i )';
    } else {
      this.title = this.title + ' ( Ctrl + i )';
      this.shortcut = 'ctrl+i';
    }
    return ItalicButton.__super__._init.call(this);
  };

  ItalicButton.prototype.status = function($node) {
    var active;
    if ($node != null) {
      this.setDisabled($node.is(this.disableTag));
    }
    if (this.disabled) {
      return this.disabled;
    }
    active = document.queryCommandState('italic') === true;
    this.setActive(active);
    return active;
  };

  ItalicButton.prototype.command = function() {
    document.execCommand('italic');
    this.editor.trigger('valuechanged');
    return $(document).trigger('selectionchange');
  };

  return ItalicButton;

})(Button);

Simditor.Toolbar.addButton(ItalicButton);

var UnderlineButton,
  __hasProp = {}.hasOwnProperty,
  __extends = function(child, parent) { for (var key in parent) { if (__hasProp.call(parent, key)) child[key] = parent[key]; } function ctor() { this.constructor = child; } ctor.prototype = parent.prototype; child.prototype = new ctor(); child.__super__ = parent.prototype; return child; };

UnderlineButton = (function(_super) {
  __extends(UnderlineButton, _super);

  function UnderlineButton() {
    return UnderlineButton.__super__.constructor.apply(this, arguments);
  }

  UnderlineButton.prototype.name = 'underline';

  UnderlineButton.prototype.icon = 'underline';

  UnderlineButton.prototype.htmlTag = 'u';

  UnderlineButton.prototype.disableTag = 'pre';

  UnderlineButton.prototype.shortcut = 'cmd+u';

  UnderlineButton.prototype.render = function() {
    if (this.editor.util.os.mac) {
      this.title = this.title + ' ( Cmd + u )';
    } else {
      this.title = this.title + ' ( Ctrl + u )';
      this.shortcut = 'ctrl+u';
    }
    return UnderlineButton.__super__.render.call(this);
  };

  UnderlineButton.prototype.status = function($node) {
    var active;
    if ($node != null) {
      this.setDisabled($node.is(this.disableTag));
    }
    if (this.disabled) {
      return this.disabled;
    }
    active = document.queryCommandState('underline') === true;
    this.setActive(active);
    return active;
  };

  UnderlineButton.prototype.command = function() {
    document.execCommand('underline');
    this.editor.trigger('valuechanged');
    return $(document).trigger('selectionchange');
  };

  return UnderlineButton;

})(Button);

Simditor.Toolbar.addButton(UnderlineButton);

var ColorButton,
  __hasProp = {}.hasOwnProperty,
  __extends = function(child, parent) { for (var key in parent) { if (__hasProp.call(parent, key)) child[key] = parent[key]; } function ctor() { this.constructor = child; } ctor.prototype = parent.prototype; child.prototype = new ctor(); child.__super__ = parent.prototype; return child; },
  __slice = [].slice;

ColorButton = (function(_super) {
  __extends(ColorButton, _super);

  function ColorButton() {
    return ColorButton.__super__.constructor.apply(this, arguments);
  }

  ColorButton.prototype.name = 'color';

  ColorButton.prototype.icon = 'font';

  ColorButton.prototype.disableTag = 'pre';

  ColorButton.prototype.menu = true;

  ColorButton.prototype.render = function() {
    var args;
    args = 1 <= arguments.length ? __slice.call(arguments, 0) : [];
    return ColorButton.__super__.render.apply(this, args);
  };

  ColorButton.prototype.renderMenu = function() {
    $('<ul class="color-list">\n  <li><a href="javascript:;" class="font-color font-color-1" data-color=""></a></li>\n  <li><a href="javascript:;" class="font-color font-color-2" data-color=""></a></li>\n  <li><a href="javascript:;" class="font-color font-color-3" data-color=""></a></li>\n  <li><a href="javascript:;" class="font-color font-color-4" data-color=""></a></li>\n  <li><a href="javascript:;" class="font-color font-color-5" data-color=""></a></li>\n  <li><a href="javascript:;" class="font-color font-color-6" data-color=""></a></li>\n  <li><a href="javascript:;" class="font-color font-color-7" data-color=""></a></li>\n  <li><a href="javascript:;" class="font-color font-color-default" data-color=""></a></li>\n</ul>').appendTo(this.menuWrapper);
    this.menuWrapper.on('mousedown', '.color-list', function(e) {
      return false;
    });
    return this.menuWrapper.on('click', '.font-color', (function(_this) {
      return function(e) {
        var $link, $p, hex, rgb;
        _this.wrapper.removeClass('menu-on');
        $link = $(e.currentTarget);
        if ($link.hasClass('font-color-default')) {
          $p = _this.editor.body.find('p, li');
          if (!($p.length > 0)) {
            return;
          }
          rgb = window.getComputedStyle($p[0], null).getPropertyValue('color');
          hex = _this._convertRgbToHex(rgb);
        } else {
          rgb = window.getComputedStyle($link[0], null).getPropertyValue('background-color');
          hex = _this._convertRgbToHex(rgb);
        }
        if (!hex) {
          return;
        }
        document.execCommand('foreColor', false, hex);
        return _this.editor.trigger('valuechanged');
      };
    })(this));
  };

  ColorButton.prototype._convertRgbToHex = function(rgb) {
    var match, re, rgbToHex;
    re = /rgb\((\d+),\s?(\d+),\s?(\d+)\)/g;
    match = re.exec(rgb);
    if (!match) {
      return '';
    }
    rgbToHex = function(r, g, b) {
      var componentToHex;
      componentToHex = function(c) {
        var hex;
        hex = c.toString(16);
        if (hex.length === 1) {
          return '0' + hex;
        } else {
          return hex;
        }
      };
      return "#" + componentToHex(r) + componentToHex(g) + componentToHex(b);
    };
    return rgbToHex(match[1] * 1, match[2] * 1, match[3] * 1);
  };

  return ColorButton;

})(Button);

Simditor.Toolbar.addButton(ColorButton);

var ListButton, OrderListButton, UnorderListButton,
  __hasProp = {}.hasOwnProperty,
  __extends = function(child, parent) { for (var key in parent) { if (__hasProp.call(parent, key)) child[key] = parent[key]; } function ctor() { this.constructor = child; } ctor.prototype = parent.prototype; child.prototype = new ctor(); child.__super__ = parent.prototype; return child; };

ListButton = (function(_super) {
  __extends(ListButton, _super);

  function ListButton() {
    return ListButton.__super__.constructor.apply(this, arguments);
  }

  ListButton.prototype.type = '';

  ListButton.prototype.disableTag = 'pre, table';

  ListButton.prototype.status = function($node) {
    var anotherType;
    if ($node != null) {
      this.setDisabled($node.is(this.disableTag));
    }
    if (this.disabled) {
      return true;
    }
    if ($node == null) {
      return this.active;
    }
    anotherType = this.type === 'ul' ? 'ol' : 'ul';
    if ($node.is(anotherType)) {
      this.setActive(false);
      return true;
    } else {
      this.setActive($node.is(this.htmlTag));
      return this.active;
    }
  };

  ListButton.prototype.command = function(param) {
    var $contents, $endBlock, $furthestEnd, $furthestStart, $parent, $startBlock, endLevel, endNode, getListLevel, node, range, results, startLevel, startNode, _i, _len, _ref;
    range = this.editor.selection.getRange();
    startNode = range.startContainer;
    endNode = range.endContainer;
    $startBlock = this.editor.util.closestBlockEl(startNode);
    $endBlock = this.editor.util.closestBlockEl(endNode);
    this.editor.selection.save();
    range.setStartBefore($startBlock[0]);
    range.setEndAfter($endBlock[0]);
    if ($startBlock.is('li') && $endBlock.is('li')) {
      $furthestStart = this.editor.util.furthestNode($startBlock, 'ul, ol');
      $furthestEnd = this.editor.util.furthestNode($endBlock, 'ul, ol');
      if ($furthestStart.is($furthestEnd)) {
        getListLevel = function($li) {
          var lvl;
          lvl = 1;
          while (!$li.parent().is($furthestStart)) {
            lvl += 1;
            $li = $li.parent();
          }
          return lvl;
        };
        startLevel = getListLevel($startBlock);
        endLevel = getListLevel($endBlock);
        if (startLevel > endLevel) {
          $parent = $endBlock.parent();
        } else {
          $parent = $startBlock.parent();
        }
        range.setStartBefore($parent[0]);
        range.setEndAfter($parent[0]);
      } else {
        range.setStartBefore($furthestStart[0]);
        range.setEndAfter($furthestEnd[0]);
      }
    }
    $contents = $(range.extractContents());
    results = [];
    $contents.children().each((function(_this) {
      return function(i, el) {
        var c, converted, _i, _len, _results;
        converted = _this._convertEl(el);
        _results = [];
        for (_i = 0, _len = converted.length; _i < _len; _i++) {
          c = converted[_i];
          if (results.length && results[results.length - 1].is(_this.type) && c.is(_this.type)) {
            _results.push(results[results.length - 1].append(c.children()));
          } else {
            _results.push(results.push(c));
          }
        }
        return _results;
      };
    })(this));
    _ref = results.reverse();
    for (_i = 0, _len = _ref.length; _i < _len; _i++) {
      node = _ref[_i];
      range.insertNode(node[0]);
    }
    this.editor.selection.restore();
    return this.editor.trigger('valuechanged');
  };

  ListButton.prototype._convertEl = function(el) {
    var $el, anotherType, block, child, children, results, _i, _len, _ref;
    $el = $(el);
    results = [];
    anotherType = this.type === 'ul' ? 'ol' : 'ul';
    if ($el.is(this.type)) {
      $el.children('li').each((function(_this) {
        return function(i, li) {
          var $childList, $li, block;
          $li = $(li);
          $childList = $li.children('ul, ol').remove();
          block = $('<p/>').append($(li).html() || _this.editor.util.phBr);
          results.push(block);
          if ($childList.length > 0) {
            return results.push($childList);
          }
        };
      })(this));
    } else if ($el.is(anotherType)) {
      block = $('<' + this.type + '/>').append($el.html());
      results.push(block);
    } else if ($el.is('blockquote')) {
      _ref = $el.children().get();
      for (_i = 0, _len = _ref.length; _i < _len; _i++) {
        child = _ref[_i];
        children = this._convertEl(child);
      }
      $.merge(results, children);
    } else if ($el.is('table')) {

    } else {
      block = $('<' + this.type + '><li></li></' + this.type + '>');
      block.find('li').append($el.html() || this.editor.util.phBr);
      results.push(block);
    }
    return results;
  };

  return ListButton;

})(Button);

OrderListButton = (function(_super) {
  __extends(OrderListButton, _super);

  function OrderListButton() {
    return OrderListButton.__super__.constructor.apply(this, arguments);
  }

  OrderListButton.prototype.type = 'ol';

  OrderListButton.prototype.name = 'ol';

  OrderListButton.prototype.icon = 'list-ol';

  OrderListButton.prototype.htmlTag = 'ol';

  OrderListButton.prototype.shortcut = 'cmd+/';

  OrderListButton.prototype._init = function() {
    if (this.editor.util.os.mac) {
      this.title = this.title + ' ( Cmd + / )';
    } else {
      this.title = this.title + ' ( ctrl + / )';
      this.shortcut = 'ctrl+/';
    }
    return OrderListButton.__super__._init.call(this);
  };

  return OrderListButton;

})(ListButton);

UnorderListButton = (function(_super) {
  __extends(UnorderListButton, _super);

  function UnorderListButton() {
    return UnorderListButton.__super__.constructor.apply(this, arguments);
  }

  UnorderListButton.prototype.type = 'ul';

  UnorderListButton.prototype.name = 'ul';

  UnorderListButton.prototype.icon = 'list-ul';

  UnorderListButton.prototype.htmlTag = 'ul';

  UnorderListButton.prototype.shortcut = 'cmd+.';

  UnorderListButton.prototype._init = function() {
    if (this.editor.util.os.mac) {
      this.title = this.title + ' ( Cmd + . )';
    } else {
      this.title = this.title + ' ( Ctrl + . )';
      this.shortcut = 'ctrl+.';
    }
    return UnorderListButton.__super__._init.call(this);
  };

  return UnorderListButton;

})(ListButton);

Simditor.Toolbar.addButton(OrderListButton);

Simditor.Toolbar.addButton(UnorderListButton);

var BlockquoteButton,
  __hasProp = {}.hasOwnProperty,
  __extends = function(child, parent) { for (var key in parent) { if (__hasProp.call(parent, key)) child[key] = parent[key]; } function ctor() { this.constructor = child; } ctor.prototype = parent.prototype; child.prototype = new ctor(); child.__super__ = parent.prototype; return child; };

BlockquoteButton = (function(_super) {
  __extends(BlockquoteButton, _super);

  function BlockquoteButton() {
    return BlockquoteButton.__super__.constructor.apply(this, arguments);
  }

  BlockquoteButton.prototype.name = 'blockquote';

  BlockquoteButton.prototype.icon = 'quote-left';

  BlockquoteButton.prototype.htmlTag = 'blockquote';

  BlockquoteButton.prototype.disableTag = 'pre, table';

  BlockquoteButton.prototype.command = function() {
    var $contents, $endBlock, $startBlock, endNode, node, range, results, startNode, _i, _len, _ref;
    range = this.editor.selection.getRange();
    startNode = range.startContainer;
    endNode = range.endContainer;
    $startBlock = this.editor.util.furthestBlockEl(startNode);
    $endBlock = this.editor.util.furthestBlockEl(endNode);
    this.editor.selection.save();
    range.setStartBefore($startBlock[0]);
    range.setEndAfter($endBlock[0]);
    $contents = $(range.extractContents());
    results = [];
    $contents.children().each((function(_this) {
      return function(i, el) {
        var c, converted, _i, _len, _results;
        converted = _this._convertEl(el);
        _results = [];
        for (_i = 0, _len = converted.length; _i < _len; _i++) {
          c = converted[_i];
          if (results.length && results[results.length - 1].is(_this.htmlTag) && c.is(_this.htmlTag)) {
            _results.push(results[results.length - 1].append(c.children()));
          } else {
            _results.push(results.push(c));
          }
        }
        return _results;
      };
    })(this));
    _ref = results.reverse();
    for (_i = 0, _len = _ref.length; _i < _len; _i++) {
      node = _ref[_i];
      range.insertNode(node[0]);
    }
    this.editor.selection.restore();
    return this.editor.trigger('valuechanged');
  };

  BlockquoteButton.prototype._convertEl = function(el) {
    var $el, block, results;
    $el = $(el);
    results = [];
    if ($el.is(this.htmlTag)) {
      $el.children().each((function(_this) {
        return function(i, node) {
          return results.push($(node));
        };
      })(this));
    } else {
      block = $('<' + this.htmlTag + '/>').append($el);
      results.push(block);
    }
    return results;
  };

  return BlockquoteButton;

})(Button);

Simditor.Toolbar.addButton(BlockquoteButton);

var CodeButton, CodePopover,
  __hasProp = {}.hasOwnProperty,
  __extends = function(child, parent) { for (var key in parent) { if (__hasProp.call(parent, key)) child[key] = parent[key]; } function ctor() { this.constructor = child; } ctor.prototype = parent.prototype; child.prototype = new ctor(); child.__super__ = parent.prototype; return child; },
  __slice = [].slice;

CodeButton = (function(_super) {
  __extends(CodeButton, _super);

  function CodeButton() {
    return CodeButton.__super__.constructor.apply(this, arguments);
  }

  CodeButton.prototype.name = 'code';

  CodeButton.prototype.icon = 'code';

  CodeButton.prototype.htmlTag = 'pre';

  CodeButton.prototype.disableTag = 'li, table';

  CodeButton.prototype._init = function() {
    CodeButton.__super__._init.call(this);
    this.editor.on('decorate', (function(_this) {
      return function(e, $el) {
        return $el.find('pre').each(function(i, pre) {
          return _this.decorate($(pre));
        });
      };
    })(this));
    return this.editor.on('undecorate', (function(_this) {
      return function(e, $el) {
        return $el.find('pre').each(function(i, pre) {
          return _this.undecorate($(pre));
        });
      };
    })(this));
  };

  CodeButton.prototype.render = function() {
    var args;
    args = 1 <= arguments.length ? __slice.call(arguments, 0) : [];
    CodeButton.__super__.render.apply(this, args);
    return this.popover = new CodePopover({
      button: this
    });
  };

  CodeButton.prototype.status = function($node) {
    var result;
    result = CodeButton.__super__.status.call(this, $node);
    if (this.active) {
      this.popover.show($node);
    } else if (this.editor.util.isBlockNode($node)) {
      this.popover.hide();
    }
    return result;
  };

  CodeButton.prototype.decorate = function($pre) {
    var lang;
    lang = $pre.attr('data-lang');
    $pre.removeClass();
    if (lang && lang !== -1) {
      return $pre.addClass('lang-' + lang);
    }
  };

  CodeButton.prototype.undecorate = function($pre) {
    var lang;
    lang = $pre.attr('data-lang');
    $pre.removeClass();
    if (lang && lang !== -1) {
      return $pre.addClass('lang-' + lang);
    }
  };

  CodeButton.prototype.command = function() {
    var $contents, $endBlock, $startBlock, endNode, node, range, results, startNode, _i, _len, _ref;
    range = this.editor.selection.getRange();
    startNode = range.startContainer;
    endNode = range.endContainer;
    $startBlock = this.editor.util.closestBlockEl(startNode);
    $endBlock = this.editor.util.closestBlockEl(endNode);
    range.setStartBefore($startBlock[0]);
    range.setEndAfter($endBlock[0]);
    $contents = $(range.extractContents());
    results = [];
    $contents.children().each((function(_this) {
      return function(i, el) {
        var c, converted, _i, _len, _results;
        converted = _this._convertEl(el);
        _results = [];
        for (_i = 0, _len = converted.length; _i < _len; _i++) {
          c = converted[_i];
          if (results.length && results[results.length - 1].is(_this.htmlTag) && c.is(_this.htmlTag)) {
            _results.push(results[results.length - 1].append(c.contents()));
          } else {
            _results.push(results.push(c));
          }
        }
        return _results;
      };
    })(this));
    _ref = results.reverse();
    for (_i = 0, _len = _ref.length; _i < _len; _i++) {
      node = _ref[_i];
      range.insertNode(node[0]);
    }
    this.editor.selection.setRangeAtEndOf(results[0]);
    return this.editor.trigger('valuechanged');
  };

  CodeButton.prototype._convertEl = function(el) {
    var $el, block, codeStr, results;
    $el = $(el);
    results = [];
    if ($el.is(this.htmlTag)) {
      block = $('<p/>').append($el.html().replace('\n', '<br/>'));
      results.push(block);
    } else {
      if (!$el.text() && $el.children().length === 1 && $el.children().is('br')) {
        codeStr = '\n';
      } else {
        codeStr = this.editor.formatter.clearHtml($el);
      }
      block = $('<' + this.htmlTag + '/>').text(codeStr);
      results.push(block);
    }
    return results;
  };

  return CodeButton;

})(Button);

CodePopover = (function(_super) {
  __extends(CodePopover, _super);

  function CodePopover() {
    return CodePopover.__super__.constructor.apply(this, arguments);
  }

  CodePopover.prototype._tpl = "<div class=\"code-settings\">\n  <div class=\"settings-field\">\n    <select class=\"select-lang\">\n      <option value=\"-1\">选择程序语言</option>\n      <option value=\"bash\">Bash</option>\n      <option value=\"c++\">C++</option>\n      <option value=\"cs\">C#</option>\n      <option value=\"css\">CSS</option>\n      <option value=\"erlang\">Erlang</option>\n      <option value=\"less\">Less</option>\n      <option value=\"scss\">Sass</option>\n      <option value=\"diff\">Diff</option>\n      <option value=\"coffeeScript\">CoffeeScript</option>\n      <option value=\"html\">Html,XML</option>\n      <option value=\"json\">JSON</option>\n      <option value=\"java\">Java</option>\n      <option value=\"js\">JavaScript</option>\n      <option value=\"markdown\">Markdown</option>\n      <option value=\"oc\">Objective C</option>\n      <option value=\"php\">PHP</option>\n      <option value=\"perl\">Perl</option>\n      <option value=\"python\">Python</option>\n      <option value=\"ruby\">Ruby</option>\n      <option value=\"sql\">SQL</option>\n    </select>\n  </div>\n</div>";

  CodePopover.prototype.render = function() {
    this.el.addClass('code-popover').append(this._tpl);
    this.selectEl = this.el.find('.select-lang');
    return this.selectEl.on('change', (function(_this) {
      return function(e) {
        var selected;
        _this.lang = _this.selectEl.val();
        selected = _this.target.hasClass('selected');
        _this.target.removeClass().removeAttr('data-lang');
        if (_this.lang !== -1) {
          _this.target.addClass('lang-' + _this.lang);
          _this.target.attr('data-lang', _this.lang);
        }
        if (selected) {
          return _this.target.addClass('selected');
        }
      };
    })(this));
  };

  CodePopover.prototype.show = function() {
    var args;
    args = 1 <= arguments.length ? __slice.call(arguments, 0) : [];
    CodePopover.__super__.show.apply(this, args);
    this.lang = this.target.attr('data-lang');
    if (this.lang != null) {
      return this.selectEl.val(this.lang);
    } else {
      return this.selectEl.val(-1);
    }
  };

  return CodePopover;

})(Popover);

Simditor.Toolbar.addButton(CodeButton);

var LinkButton, LinkPopover,
  __hasProp = {}.hasOwnProperty,
  __extends = function(child, parent) { for (var key in parent) { if (__hasProp.call(parent, key)) child[key] = parent[key]; } function ctor() { this.constructor = child; } ctor.prototype = parent.prototype; child.prototype = new ctor(); child.__super__ = parent.prototype; return child; },
  __slice = [].slice;

LinkButton = (function(_super) {
  __extends(LinkButton, _super);

  function LinkButton() {
    return LinkButton.__super__.constructor.apply(this, arguments);
  }

  LinkButton.prototype.name = 'link';

  LinkButton.prototype.icon = 'link';

  LinkButton.prototype.htmlTag = 'a';

  LinkButton.prototype.disableTag = 'pre';

  LinkButton.prototype.render = function() {
    var args;
    args = 1 <= arguments.length ? __slice.call(arguments, 0) : [];
    LinkButton.__super__.render.apply(this, args);
    return this.popover = new LinkPopover({
      button: this
    });
  };

  LinkButton.prototype.status = function($node) {
    var showPopover;
    if ($node != null) {
      this.setDisabled($node.is(this.disableTag));
    }
    if (this.disabled) {
      return true;
    }
    if ($node == null) {
      return this.active;
    }
    showPopover = true;
    if (!$node.is(this.htmlTag) || $node.is('[class^="simditor-"]')) {
      this.setActive(false);
      showPopover = false;
    } else if (this.editor.selection.rangeAtEndOf($node)) {
      this.setActive(true);
      showPopover = false;
    } else {
      this.setActive(true);
    }
    if (showPopover) {
      this.popover.show($node);
    } else if (this.editor.util.isBlockNode($node)) {
      this.popover.hide();
    }
    return this.active;
  };

  LinkButton.prototype.command = function() {
    var $contents, $endBlock, $link, $newBlock, $startBlock, endNode, linkText, range, startNode, txtNode;
    range = this.editor.selection.getRange();
    if (this.active) {
      $link = $(range.commonAncestorContainer).closest('a');
      txtNode = document.createTextNode($link.text());
      $link.replaceWith(txtNode);
      range.selectNode(txtNode);
    } else {
      startNode = range.startContainer;
      endNode = range.endContainer;
      $startBlock = this.editor.util.closestBlockEl(startNode);
      $endBlock = this.editor.util.closestBlockEl(endNode);
      $contents = $(range.extractContents());
      linkText = this.editor.formatter.clearHtml($contents.contents(), false);
      $link = $('<a/>', {
        href: 'http://www.example.com',
        target: '_blank',
        text: linkText || this._t('linkText')
      });
      if ($startBlock[0] === $endBlock[0]) {
        range.insertNode($link[0]);
      } else {
        $newBlock = $('<p/>').append($link);
        range.insertNode($newBlock[0]);
      }
      range.selectNodeContents($link[0]);
      this.popover.one('popovershow', (function(_this) {
        return function() {
          if (linkText) {
            _this.popover.urlEl.focus();
            return _this.popover.urlEl[0].select();
          } else {
            _this.popover.textEl.focus();
            return _this.popover.textEl[0].select();
          }
        };
      })(this));
    }
    this.editor.selection.selectRange(range);
    return this.editor.trigger('valuechanged');
  };

  return LinkButton;

})(Button);

LinkPopover = (function(_super) {
  __extends(LinkPopover, _super);

  function LinkPopover() {
    return LinkPopover.__super__.constructor.apply(this, arguments);
  }

  LinkPopover.prototype.render = function() {
    var tpl;
    tpl = "<div class=\"link-settings\">\n  <div class=\"settings-field\">\n    <label>" + (this._t('text')) + "</label>\n    <input class=\"link-text\" type=\"text\"/>\n    <a class=\"btn-unlink\" href=\"javascript:;\" title=\"" + (this._t('removeLink')) + "\" tabindex=\"-1\"><span class=\"fa fa-unlink\"></span></a>\n  </div>\n  <div class=\"settings-field\">\n    <label>" + (this._t('linkUrl')) + "</label>\n    <input class=\"link-url\" type=\"text\"/>\n  </div>\n</div>";
    this.el.addClass('link-popover').append(tpl);
    this.textEl = this.el.find('.link-text');
    this.urlEl = this.el.find('.link-url');
    this.unlinkEl = this.el.find('.btn-unlink');
    this.textEl.on('keyup', (function(_this) {
      return function(e) {
        if (e.which === 13) {
          return;
        }
        return _this.target.text(_this.textEl.val());
      };
    })(this));
    this.urlEl.on('keyup', (function(_this) {
      return function(e) {
        var val;
        if (e.which === 13) {
          return;
        }
        val = _this.urlEl.val();
        if (!(/https?:\/\/|^\//ig.test(val) || !val)) {
          val = 'http://' + val;
        }
        return _this.target.attr('href', val);
      };
    })(this));
    $([this.urlEl[0], this.textEl[0]]).on('keydown', (function(_this) {
      return function(e) {
        if (e.which === 13 || e.which === 27 || (!e.shiftKey && e.which === 9 && $(e.target).hasClass('link-url'))) {
          e.preventDefault();
          return setTimeout(function() {
            var range;
            range = document.createRange();
            _this.editor.selection.setRangeAfter(_this.target, range);
            _this.hide();
            return _this.editor.trigger('valuechanged');
          }, 0);
        }
      };
    })(this));
    return this.unlinkEl.on('click', (function(_this) {
      return function(e) {
        var range, txtNode;
        txtNode = document.createTextNode(_this.target.text());
        _this.target.replaceWith(txtNode);
        _this.hide();
        range = document.createRange();
        _this.editor.selection.setRangeAfter(txtNode, range);
        return _this.editor.trigger('valuechanged');
      };
    })(this));
  };

  LinkPopover.prototype.show = function() {
    var args;
    args = 1 <= arguments.length ? __slice.call(arguments, 0) : [];
    LinkPopover.__super__.show.apply(this, args);
    this.textEl.val(this.target.text());
    return this.urlEl.val(this.target.attr('href'));
  };

  return LinkPopover;

})(Popover);

Simditor.Toolbar.addButton(LinkButton);

var ImageButton, ImagePopover,
  __hasProp = {}.hasOwnProperty,
  __extends = function(child, parent) { for (var key in parent) { if (__hasProp.call(parent, key)) child[key] = parent[key]; } function ctor() { this.constructor = child; } ctor.prototype = parent.prototype; child.prototype = new ctor(); child.__super__ = parent.prototype; return child; },
  __slice = [].slice;

ImageButton = (function(_super) {
  __extends(ImageButton, _super);

  function ImageButton() {
    return ImageButton.__super__.constructor.apply(this, arguments);
  }

  ImageButton.prototype.name = 'image';

  ImageButton.prototype.icon = 'picture-o';

  ImageButton.prototype.htmlTag = 'img';

  ImageButton.prototype.disableTag = 'pre, table';

  ImageButton.prototype.defaultImage = '';

  ImageButton.prototype.needFocus = false;

  ImageButton.prototype._init = function() {
    if (this.editor.uploader != null) {
      this.menu = [
        {
          name: 'upload-image',
          text: this._t('localImage')
        }, {
          name: 'external-image',
          text: this._t('externalImage')
        }
      ];
    } else {
      this.menu = false;
    }
    this.defaultImage = this.editor.opts.defaultImage;
    this.editor.body.on('click', 'img:not([data-non-image])', (function(_this) {
      return function(e) {
        var $img, range;
        $img = $(e.currentTarget);
        range = document.createRange();
        range.selectNode($img[0]);
        _this.editor.selection.selectRange(range);
        if (!_this.editor.util.supportSelectionChange) {
          _this.editor.trigger('selectionchanged');
        }
        return false;
      };
    })(this));
    this.editor.body.on('mouseup', 'img:not([data-non-image])', (function(_this) {
      return function(e) {
        return false;
      };
    })(this));
    this.editor.on('selectionchanged.image', (function(_this) {
      return function() {
        var $contents, $img, range;
        range = _this.editor.selection.getRange();
        if (range == null) {
          return;
        }
        $contents = $(range.cloneContents()).contents();
        if ($contents.length === 1 && $contents.is('img:not([data-non-image])')) {
          $img = $(range.startContainer).contents().eq(range.startOffset);
          return _this.popover.show($img);
        } else {
          return _this.popover.hide();
        }
      };
    })(this));
    this.editor.on('valuechanged.image', (function(_this) {
      return function() {
        var $masks;
        $masks = _this.editor.wrapper.find('.simditor-image-loading');
        if (!($masks.length > 0)) {
          return;
        }
        return $masks.each(function(i, mask) {
          var $img, $mask, file;
          $mask = $(mask);
          $img = $mask.data('img');
          if (!($img && $img.parent().length > 0)) {
            $mask.remove();
            if ($img) {
              file = $img.data('file');
              if (file) {
                _this.editor.uploader.cancel(file);
                if (_this.editor.body.find('img.uploading').length < 1) {
                  return _this.editor.uploader.trigger('uploadready', [file]);
                }
              }
            }
          }
        });
      };
    })(this));
    return ImageButton.__super__._init.call(this);
  };

  ImageButton.prototype.render = function() {
    var args;
    args = 1 <= arguments.length ? __slice.call(arguments, 0) : [];
    ImageButton.__super__.render.apply(this, args);
    return this.popover = new ImagePopover({
      button: this
    });
  };

  ImageButton.prototype.renderMenu = function() {
    var $input, $uploadItem, createInput;
    ImageButton.__super__.renderMenu.call(this);
    $uploadItem = this.menuEl.find('.menu-item-upload-image');
    $input = null;
    createInput = (function(_this) {
      return function() {
        if ($input) {
          $input.remove();
        }
        return $input = $('<input type="file" title="' + _this._t('uploadImage') + '" accept="image/*">').appendTo($uploadItem);
      };
    })(this);
    createInput();
    $uploadItem.on('click mousedown', 'input[type=file]', (function(_this) {
      return function(e) {
        return e.stopPropagation();
      };
    })(this));
    $uploadItem.on('change', 'input[type=file]', (function(_this) {
      return function(e) {
        if (_this.editor.inputManager.focused) {
          _this.editor.uploader.upload($input, {
            inline: true
          });
          createInput();
        } else {
          _this.editor.one('focus', function(e) {
            _this.editor.uploader.upload($input, {
              inline: true
            });
            return createInput();
          });
          _this.editor.focus();
        }
        return _this.wrapper.removeClass('menu-on');
      };
    })(this));
    return this._initUploader();
  };

  ImageButton.prototype._initUploader = function() {
    if (this.editor.uploader == null) {
      this.el.find('.btn-upload').remove();
      return;
    }
    this.editor.uploader.on('beforeupload', (function(_this) {
      return function(e, file) {
        var $img;
        if (!file.inline) {
          return;
        }
        if (file.img) {
          $img = $(file.img);
        } else {
          $img = _this.createImage(file.name);
          file.img = $img;
        }
        $img.addClass('uploading');
        $img.data('file', file);
        return _this.editor.uploader.readImageFile(file.obj, function(img) {
          var src;
          if (!$img.hasClass('uploading')) {
            return;
          }
          src = img ? img.src : _this.defaultImage;
          return _this.loadImage($img, src, function() {
            if (_this.popover.active) {
              _this.popover.refresh();
              return _this.popover.srcEl.val(_this._t('uploading')).prop('disabled', true);
            }
          });
        });
      };
    })(this));
    this.editor.uploader.on('uploadprogress', (function(_this) {
      return function(e, file, loaded, total) {
        var $img, $mask, $txt, percent;
        if (!file.inline) {
          return;
        }
        percent = loaded / total;
        percent = (percent * 100).toFixed(0);
        if (percent > 99) {
          percent = 99;
        }
        $mask = file.img.data('mask');
        if ($mask) {
          $img = $mask.data('img');
          $txt = $mask.find('span');
          if ($img && $img.parent().length > 0 && percent !== $txt.text()) {
            return $txt.text(percent);
          } else {
            return $mask.remove();
          }
        }
      };
    })(this));
    this.editor.uploader.on('uploadsuccess', (function(_this) {
      return function(e, file, result) {
        var $img, $mask, msg;
        if (!file.inline) {
          return;
        }
        $img = file.img;
        $img.removeData('file');
        $img.removeClass('uploading');
        $mask = $img.data('mask');
        if ($mask) {
          $mask.remove();
        }
        $img.removeData('mask');
        if (result.success === false) {
          msg = result.msg || _this._t('uploadFailed');
          alert(msg);
          $img.attr('src', _this.defaultImage);
        } else {
          $img.attr('src', result.file_path);
        }
        if (_this.popover.active) {
          _this.popover.srcEl.prop('disabled', false);
          _this.popover.srcEl.val(result.file_path);
        }
        _this.editor.trigger('valuechanged');
        if (_this.editor.body.find('img.uploading').length < 1) {
          return _this.editor.uploader.trigger('uploadready', [file, result]);
        }
      };
    })(this));
    return this.editor.uploader.on('uploaderror', (function(_this) {
      return function(e, file, xhr) {
        var $img, $mask, msg, result;
        if (!file.inline) {
          return;
        }
        if (xhr.statusText === 'abort') {
          return;
        }
        if (xhr.responseText) {
          try {
            result = $.parseJSON(xhr.responseText);
            msg = result.msg;
          } catch (_error) {
            e = _error;
            msg = _this._t('uploadError');
          }
          alert(msg);
        }
        $img = file.img;
        $img.removeData('file');
        $img.removeClass('uploading');
        $mask = $img.data('mask');
        if ($mask) {
          $mask.remove();
        }
        $img.removeData('mask');
        $img.attr('src', _this.defaultImage);
        if (_this.popover.active) {
          _this.popover.srcEl.prop('disabled', false);
          _this.popover.srcEl.val(_this.defaultImage);
        }
        _this.editor.trigger('valuechanged');
        if (_this.editor.body.find('img.uploading').length < 1) {
          return _this.editor.uploader.trigger('uploadready', [file, result]);
        }
      };
    })(this));
  };

  ImageButton.prototype.status = function($node) {
    if ($node != null) {
      this.setDisabled($node.is(this.disableTag));
    }
    if (this.disabled) {
      return true;
    }
  };

  ImageButton.prototype.loadImage = function($img, src, callback) {
    var $mask, img;
    $mask = $img.data('mask');
    if (!$mask) {
      $mask = $('<div class="simditor-image-loading"><span></span></div>').hide().appendTo(this.editor.wrapper);
      if ($img.hasClass('uploading')) {
        $mask.addClass('uploading');
      }
      $img.data('mask', $mask);
      $mask.data('img', $img);
    }
    img = new Image();
    img.onload = (function(_this) {
      return function() {
        var height, imgOffset, width, wrapperOffset;
        if ($mask.hasClass('uploading') && !$img.hasClass('uploading')) {
          return;
        }
        width = img.width;
        height = img.height;
        $img.attr({
          src: src,
          'data-image-size': width + ',' + height
        });
        if ($img.hasClass('uploading')) {
          _this.editor.util.reflow(_this.editor.body);
          wrapperOffset = _this.editor.wrapper.offset();
          imgOffset = $img.offset();
          $mask.css({
            top: imgOffset.top - wrapperOffset.top,
            left: imgOffset.left - wrapperOffset.left,
            width: $img.width(),
            height: $img.height()
          }).show();
        } else {
          $mask.remove();
          $img.removeData('mask');
        }
        return callback(img);
      };
    })(this);
    img.onerror = (function(_this) {
      return function() {
        callback(false);
        $mask.remove();
        return $img.removeData('mask');
      };
    })(this);
    return img.src = src;
  };

  ImageButton.prototype.createImage = function(name) {
    var $block, $img, $nextBlock, range;
    if (name == null) {
      name = 'Image';
    }
    if (!this.editor.inputManager.focused) {
      this.editor.focus();
    }
    range = this.editor.selection.getRange();
    range.deleteContents();
    $block = this.editor.util.closestBlockEl();
    if ($block.is('p') && !this.editor.util.isEmptyNode($block)) {
      $block = $('<p/>').append(this.editor.util.phBr).insertAfter($block);
      this.editor.selection.setRangeAtStartOf($block, range);
    }
    $img = $('<img/>').attr('alt', name);
    range.insertNode($img[0]);
    $nextBlock = $block.next('p');
    if (!($nextBlock.length > 0)) {
      $nextBlock = $('<p/>').append(this.editor.util.phBr).insertAfter($block);
    }
    this.editor.selection.setRangeAtStartOf($nextBlock);
    return $img;
  };

  ImageButton.prototype.command = function(src) {
    var $img;
    $img = this.createImage();
    return this.loadImage($img, src || this.defaultImage, (function(_this) {
      return function() {
        _this.editor.trigger('valuechanged');
        _this.editor.util.reflow($img);
        $img.click();
        return _this.popover.one('popovershow', function() {
          _this.popover.srcEl.focus();
          return _this.popover.srcEl[0].select();
        });
      };
    })(this));
  };

  return ImageButton;

})(Button);

ImagePopover = (function(_super) {
  __extends(ImagePopover, _super);

  function ImagePopover() {
    return ImagePopover.__super__.constructor.apply(this, arguments);
  }

  ImagePopover.prototype.offset = {
    top: 6,
    left: -4
  };

  ImagePopover.prototype.render = function() {
    var tpl;
    tpl = "<div class=\"link-settings\">\n  <div class=\"settings-field\">\n    <label>" + (this._t('imageUrl')) + "</label>\n    <input class=\"image-src\" type=\"text\" tabindex=\"1\" />\n    <a class=\"btn-upload\" href=\"javascript:;\" title=\"" + (this._t('uploadImage')) + "\" tabindex=\"-1\">\n      <span class=\"fa fa-upload\"></span>\n    </a>\n  </div>\n  <div class=\"settings-field\">\n    <label>" + (this._t('imageSize')) + "</label>\n    <input class=\"image-size\" id=\"image-width\" type=\"text\" tabindex=\"2\" />\n    <span class=\"times\">×</span>\n    <input class=\"image-size\" id=\"image-height\" type=\"text\" tabindex=\"3\" />\n    <a class=\"btn-restore\" href=\"javascript:;\" title=\"" + (this._t('restoreImageSize')) + "\" tabindex=\"-1\">\n      <span class=\"fa fa-reply\"></span>\n    </a>\n  </div>\n</div>";
    this.el.addClass('image-popover').append(tpl);
    this.srcEl = this.el.find('.image-src');
    this.srcEl.on('keydown', (function(_this) {
      return function(e) {
        var hideAndFocus, src;
        if (!(e.which === 13 || e.which === 27)) {
          return;
        }
        e.preventDefault();
        hideAndFocus = function() {
          _this.button.editor.body.focus();
          _this.button.editor.selection.setRangeAfter(_this.target);
          return _this.hide();
        };
        if (e.which === 13 && !_this.target.hasClass('uploading')) {
          src = _this.srcEl.val();
          if (/^data:image/.test(src) && !_this.editor.uploader) {
            hideAndFocus();
            return;
          }
          return _this.button.loadImage(_this.target, src, function(success) {
            var blob;
            if (!success) {
              return;
            }
            if (/^data:image/.test(src)) {
              blob = _this.editor.util.dataURLtoBlob(src);
              blob.name = "Base64 Image.png";
              return _this.editor.uploader.upload(blob, {
                inline: true,
                img: _this.target
              });
            } else {
              hideAndFocus();
              return _this.editor.trigger('valuechanged');
            }
          });
        } else {
          return hideAndFocus();
        }
      };
    })(this));
    this.widthEl = this.el.find('#image-width');
    this.heightEl = this.el.find('#image-height');
    this.el.find('.image-size').on('blur', (function(_this) {
      return function(e) {
        _this._resizeImg($(e.currentTarget));
        return _this.el.data('popover').refresh();
      };
    })(this));
    this.el.find('.image-size').on('keyup', (function(_this) {
      return function(e) {
        var inputEl;
        inputEl = $(e.currentTarget);
        if (!(e.which === 13 || e.which === 27 || e.which === 9)) {
          return _this._resizeImg(inputEl, true);
        }
      };
    })(this));
    this.el.find('.image-size').on('keydown', (function(_this) {
      return function(e) {
        var inputEl;
        inputEl = $(e.currentTarget);
        if (e.which === 13 || e.which === 27) {
          e.preventDefault();
          if (e.which === 13) {
            _this._resizeImg(inputEl);
          } else {
            _this._restoreImg();
          }
          _this.button.editor.body.focus();
          _this.button.editor.selection.setRangeAfter(_this.target);
          return _this.hide();
        } else if (e.which === 9) {
          return _this.el.data('popover').refresh();
        }
      };
    })(this));
    this.el.find('.btn-restore').on('click', (function(_this) {
      return function(e) {
        _this._restoreImg();
        return _this.el.data('popover').refresh();
      };
    })(this));
    this.editor.on('valuechanged', (function(_this) {
      return function(e) {
        if (_this.active) {
          return _this.refresh();
        }
      };
    })(this));
    return this._initUploader();
  };

  ImagePopover.prototype._initUploader = function() {
    var $uploadBtn, createInput;
    $uploadBtn = this.el.find('.btn-upload');
    if (this.editor.uploader == null) {
      $uploadBtn.remove();
      return;
    }
    createInput = (function(_this) {
      return function() {
        if (_this.input) {
          _this.input.remove();
        }
        return _this.input = $('<input type="file" title="' + _this._t('uploadImage') + '" accept="image/*">').appendTo($uploadBtn);
      };
    })(this);
    createInput();
    this.el.on('click mousedown', 'input[type=file]', (function(_this) {
      return function(e) {
        return e.stopPropagation();
      };
    })(this));
    return this.el.on('change', 'input[type=file]', (function(_this) {
      return function(e) {
        _this.editor.uploader.upload(_this.input, {
          inline: true,
          img: _this.target
        });
        return createInput();
      };
    })(this));
  };

  ImagePopover.prototype._resizeImg = function(inputEl, onlySetVal) {
    var height, value, width;
    if (onlySetVal == null) {
      onlySetVal = false;
    }
    value = inputEl.val() * 1;
    if (!($.isNumeric(value) || value < 0)) {
      return;
    }
    if (inputEl.is(this.widthEl)) {
      height = this.height * value / this.width;
      this.heightEl.val(height);
    } else {
      width = this.width * value / this.height;
      this.widthEl.val(width);
    }
    if (!onlySetVal) {
      return this.target.attr({
        width: width || value,
        height: height || value
      });
    }
  };

  ImagePopover.prototype._restoreImg = function() {
    var size, _ref;
    size = ((_ref = this.target.data('image-size')) != null ? _ref.split(",") : void 0) || [this.width, this.height];
    this.target.attr({
      width: size[0] * 1,
      height: size[1] * 1
    });
    this.widthEl.val(size[0]);
    return this.heightEl.val(size[1]);
  };

  ImagePopover.prototype.show = function() {
    var $img, args;
    args = 1 <= arguments.length ? __slice.call(arguments, 0) : [];
    ImagePopover.__super__.show.apply(this, args);
    $img = this.target;
    this.width = $img.width();
    this.height = $img.height();
    if ($img.hasClass('uploading')) {
      return this.srcEl.val(this._t('uploading')).prop('disabled', true);
    } else {
      this.srcEl.val($img.attr('src')).prop('disabled', false);
      this.widthEl.val(this.width);
      return this.heightEl.val(this.height);
    }
  };

  return ImagePopover;

})(Popover);

Simditor.Toolbar.addButton(ImageButton);

var IndentButton,
  __hasProp = {}.hasOwnProperty,
  __extends = function(child, parent) { for (var key in parent) { if (__hasProp.call(parent, key)) child[key] = parent[key]; } function ctor() { this.constructor = child; } ctor.prototype = parent.prototype; child.prototype = new ctor(); child.__super__ = parent.prototype; return child; };

IndentButton = (function(_super) {
  __extends(IndentButton, _super);

  function IndentButton() {
    return IndentButton.__super__.constructor.apply(this, arguments);
  }

  IndentButton.prototype.name = 'indent';

  IndentButton.prototype.icon = 'indent';

  IndentButton.prototype._init = function() {
    this.title = this._t(this.name) + ' (Tab)';
    return IndentButton.__super__._init.call(this);
  };

  IndentButton.prototype.status = function($node) {
    return true;
  };

  IndentButton.prototype.command = function() {
    return this.editor.util.indent();
  };

  return IndentButton;

})(Button);

Simditor.Toolbar.addButton(IndentButton);

var OutdentButton,
  __hasProp = {}.hasOwnProperty,
  __extends = function(child, parent) { for (var key in parent) { if (__hasProp.call(parent, key)) child[key] = parent[key]; } function ctor() { this.constructor = child; } ctor.prototype = parent.prototype; child.prototype = new ctor(); child.__super__ = parent.prototype; return child; };

OutdentButton = (function(_super) {
  __extends(OutdentButton, _super);

  function OutdentButton() {
    return OutdentButton.__super__.constructor.apply(this, arguments);
  }

  OutdentButton.prototype.name = 'outdent';

  OutdentButton.prototype.icon = 'outdent';

  OutdentButton.prototype._init = function() {
    this.title = this._t(this.name) + ' (Shift + Tab)';
    return OutdentButton.__super__._init.call(this);
  };

  OutdentButton.prototype.status = function($node) {
    return true;
  };

  OutdentButton.prototype.command = function() {
    return this.editor.util.outdent();
  };

  return OutdentButton;

})(Button);

Simditor.Toolbar.addButton(OutdentButton);

var HrButton,
  __hasProp = {}.hasOwnProperty,
  __extends = function(child, parent) { for (var key in parent) { if (__hasProp.call(parent, key)) child[key] = parent[key]; } function ctor() { this.constructor = child; } ctor.prototype = parent.prototype; child.prototype = new ctor(); child.__super__ = parent.prototype; return child; };

HrButton = (function(_super) {
  __extends(HrButton, _super);

  function HrButton() {
    return HrButton.__super__.constructor.apply(this, arguments);
  }

  HrButton.prototype.name = 'hr';

  HrButton.prototype.icon = 'minus';

  HrButton.prototype.htmlTag = 'hr';

  HrButton.prototype.status = function($node) {
    return true;
  };

  HrButton.prototype.command = function() {
    var $hr, $newBlock, $nextBlock, $rootBlock;
    $rootBlock = this.editor.util.furthestBlockEl();
    $nextBlock = $rootBlock.next();
    if ($nextBlock.length > 0) {
      this.editor.selection.save();
    } else {
      $newBlock = $('<p/>').append(this.editor.util.phBr);
    }
    $hr = $('<hr/>').insertAfter($rootBlock);
    if ($newBlock) {
      $newBlock.insertAfter($hr);
      this.editor.selection.setRangeAtStartOf($newBlock);
    } else {
      this.editor.selection.restore();
    }
    return this.editor.trigger('valuechanged');
  };

  return HrButton;

})(Button);

Simditor.Toolbar.addButton(HrButton);

var TableButton,
  __hasProp = {}.hasOwnProperty,
  __extends = function(child, parent) { for (var key in parent) { if (__hasProp.call(parent, key)) child[key] = parent[key]; } function ctor() { this.constructor = child; } ctor.prototype = parent.prototype; child.prototype = new ctor(); child.__super__ = parent.prototype; return child; };

TableButton = (function(_super) {
  __extends(TableButton, _super);

  function TableButton() {
    return TableButton.__super__.constructor.apply(this, arguments);
  }

  TableButton.prototype.name = 'table';

  TableButton.prototype.icon = 'table';

  TableButton.prototype.htmlTag = 'table';

  TableButton.prototype.disableTag = 'pre, li, blockquote';

  TableButton.prototype.menu = true;

  TableButton.prototype._init = function() {
    TableButton.__super__._init.call(this);
    $.merge(this.editor.formatter._allowedTags, ['tbody', 'tr', 'td', 'colgroup', 'col']);
    $.extend(this.editor.formatter._allowedAttributes, {
      td: ['rowspan', 'colspan'],
      col: ['width']
    });
    this._initShortcuts();
    this.editor.on('decorate', (function(_this) {
      return function(e, $el) {
        return $el.find('table').each(function(i, table) {
          return _this.decorate($(table));
        });
      };
    })(this));
    this.editor.on('undecorate', (function(_this) {
      return function(e, $el) {
        return $el.find('table').each(function(i, table) {
          return _this.undecorate($(table));
        });
      };
    })(this));
    this.editor.on('selectionchanged.table', (function(_this) {
      return function(e) {
        var $container, range;
        _this.editor.body.find('.simditor-table td').removeClass('active');
        range = _this.editor.selection.getRange();
        if (range == null) {
          return;
        }
        $container = $(range.commonAncestorContainer);
        if (range.collapsed && $container.is('.simditor-table')) {
          if (_this.editor.selection.rangeAtStartOf($container)) {
            $container = $container.find('td:first');
          } else {
            $container = $container.find('td:last');
          }
          _this.editor.selection.setRangeAtEndOf($container);
        }
        return $container.closest('td', _this.editor.body).addClass('active');
      };
    })(this));
    this.editor.on('blur.table', (function(_this) {
      return function(e) {
        return _this.editor.body.find('.simditor-table td').removeClass('active');
      };
    })(this));
    this.editor.inputManager.addKeystrokeHandler('38', 'td', (function(_this) {
      return function(e, $node) {
        var $prevTr, $tr, index;
        $tr = $node.parent('tr');
        $prevTr = $tr.prev('tr');
        if (!($prevTr.length > 0)) {
          return true;
        }
        index = $tr.find('td').index($node);
        _this.editor.selection.setRangeAtEndOf($prevTr.find('td').eq(index));
        return true;
      };
    })(this));
    return this.editor.inputManager.addKeystrokeHandler('40', 'td', (function(_this) {
      return function(e, $node) {
        var $nextTr, $tr, index;
        $tr = $node.parent('tr');
        $nextTr = $tr.next('tr');
        if (!($nextTr.length > 0)) {
          return true;
        }
        index = $tr.find('td').index($node);
        _this.editor.selection.setRangeAtEndOf($nextTr.find('td').eq(index));
        return true;
      };
    })(this));
  };

  TableButton.prototype.initResize = function($table) {
    var $colgroup, $resizeHandle, $wrapper;
    $wrapper = $table.parent('.simditor-table');
    $colgroup = $table.find('colgroup');
    if ($colgroup.length < 1) {
      $colgroup = $('<colgroup/>').prependTo($table);
      $table.find('tr:first td').each((function(_this) {
        return function(i, td) {
          var $col;
          return $col = $('<col/>').appendTo($colgroup);
        };
      })(this));
      this.refreshTableWidth($table);
    }
    $resizeHandle = $('<div class="simditor-resize-handle" contenteditable="false"></div>').appendTo($wrapper);
    $wrapper.on('mousemove', 'td', (function(_this) {
      return function(e) {
        var $col, $td, index, x, _ref, _ref1;
        if ($wrapper.hasClass('resizing')) {
          return;
        }
        $td = $(e.currentTarget);
        x = e.pageX - $(e.currentTarget).offset().left;
        if (x < 5 && $td.prev().length > 0) {
          $td = $td.prev();
        }
        if ($td.next('td').length < 1) {
          $resizeHandle.hide();
          return;
        }
        if ((_ref = $resizeHandle.data('td')) != null ? _ref.is($td) : void 0) {
          $resizeHandle.show();
          return;
        }
        index = $td.parent().find('td').index($td);
        $col = $colgroup.find('col').eq(index);
        if ((_ref1 = $resizeHandle.data('col')) != null ? _ref1.is($col) : void 0) {
          $resizeHandle.show();
          return;
        }
        return $resizeHandle.css('left', $td.position().left + $td.outerWidth() - 5).data('td', $td).data('col', $col).show();
      };
    })(this));
    $wrapper.on('mouseleave', (function(_this) {
      return function(e) {
        return $resizeHandle.hide();
      };
    })(this));
    return $wrapper.on('mousedown', '.simditor-resize-handle', (function(_this) {
      return function(e) {
        var $handle, $leftCol, $leftTd, $rightCol, $rightTd, minWidth, startHandleLeft, startLeftWidth, startRightWidth, startX, tableWidth;
        $handle = $(e.currentTarget);
        $leftTd = $handle.data('td');
        $leftCol = $handle.data('col');
        $rightTd = $leftTd.next('td');
        $rightCol = $leftCol.next('col');
        startX = e.pageX;
        startLeftWidth = $leftTd.outerWidth() * 1;
        startRightWidth = $rightTd.outerWidth() * 1;
        startHandleLeft = parseFloat($handle.css('left'));
        tableWidth = $leftTd.closest('table').width();
        minWidth = 50;
        $(document).on('mousemove.simditor-resize-table', function(e) {
          var deltaX, leftWidth, rightWidth;
          deltaX = e.pageX - startX;
          leftWidth = startLeftWidth + deltaX;
          rightWidth = startRightWidth - deltaX;
          if (leftWidth < minWidth) {
            leftWidth = minWidth;
            deltaX = minWidth - startLeftWidth;
            rightWidth = startRightWidth - deltaX;
          } else if (rightWidth < minWidth) {
            rightWidth = minWidth;
            deltaX = startRightWidth - minWidth;
            leftWidth = startLeftWidth + deltaX;
          }
          $leftCol.attr('width', (leftWidth / tableWidth * 100) + '%');
          $rightCol.attr('width', (rightWidth / tableWidth * 100) + '%');
          return $handle.css('left', startHandleLeft + deltaX);
        });
        $(document).one('mouseup.simditor-resize-table', function(e) {
          $(document).off('.simditor-resize-table');
          return $wrapper.removeClass('resizing');
        });
        $wrapper.addClass('resizing');
        return false;
      };
    })(this));
  };

  TableButton.prototype._initShortcuts = function() {
    this.editor.inputManager.addShortcut('ctrl+alt+up', (function(_this) {
      return function(e) {
        _this.editMenu.find('.menu-item[data-param=insertRowAbove]').click();
        return false;
      };
    })(this));
    this.editor.inputManager.addShortcut('ctrl+alt+down', (function(_this) {
      return function(e) {
        _this.editMenu.find('.menu-item[data-param=insertRowBelow]').click();
        return false;
      };
    })(this));
    this.editor.inputManager.addShortcut('ctrl+alt+left', (function(_this) {
      return function(e) {
        _this.editMenu.find('.menu-item[data-param=insertColLeft]').click();
        return false;
      };
    })(this));
    return this.editor.inputManager.addShortcut('ctrl+alt+right', (function(_this) {
      return function(e) {
        _this.editMenu.find('.menu-item[data-param=insertColRight]').click();
        return false;
      };
    })(this));
  };

  TableButton.prototype.decorate = function($table) {
    if ($table.parent('.simditor-table').length > 0) {
      this.undecorate($table);
    }
    $table.wrap('<div class="simditor-table"></div>');
    this.initResize($table);
    return $table.parent();
  };

  TableButton.prototype.undecorate = function($table) {
    if (!($table.parent('.simditor-table').length > 0)) {
      return;
    }
    return $table.parent().replaceWith($table);
  };

  TableButton.prototype.renderMenu = function() {
    $("<div class=\"menu-create-table\">\n</div>\n<div class=\"menu-edit-table\">\n  <ul>\n    <li><a tabindex=\"-1\" unselectable=\"on\" class=\"menu-item\" href=\"javascript:;\" data-param=\"deleteRow\"><span>" + (this._t('deleteRow')) + " ( Ctrl + Alt + → )</span></a></li>\n    <li><a tabindex=\"-1\" unselectable=\"on\" class=\"menu-item\" href=\"javascript:;\" data-param=\"insertRowAbove\"><span>" + (this._t('insertRowAbove')) + " ( Ctrl + Alt + ↑ )</span></a></li>\n    <li><a tabindex=\"-1\" unselectable=\"on\" class=\"menu-item\" href=\"javascript:;\" data-param=\"insertRowBelow\"><span>" + (this._t('insertRowBelow')) + " ( Ctrl + Alt + ↓ )</span></a></li>\n    <li><span class=\"separator\"></span></li>\n    <li><a tabindex=\"-1\" unselectable=\"on\" class=\"menu-item\" href=\"javascript:;\" data-param=\"deleteCol\"><span>" + (this._t('deleteColumn')) + "</span></a></li>\n    <li><a tabindex=\"-1\" unselectable=\"on\" class=\"menu-item\" href=\"javascript:;\" data-param=\"insertColLeft\"><span>" + (this._t('insertColumnLeft')) + " ( Ctrl + Alt + ← )</span></a></li>\n    <li><a tabindex=\"-1\" unselectable=\"on\" class=\"menu-item\" href=\"javascript:;\" data-param=\"insertColRight\"><span>" + (this._t('insertColumnRight')) + " ( Ctrl + Alt + → )</span></a></li>\n    <li><span class=\"separator\"></span></li>\n    <li><a tabindex=\"-1\" unselectable=\"on\" class=\"menu-item\" href=\"javascript:;\" data-param=\"deleteTable\"><span>" + (this._t('deleteTable')) + "</span></a></li>\n  </ul>\n</div>").appendTo(this.menuWrapper);
    this.createMenu = this.menuWrapper.find('.menu-create-table');
    this.editMenu = this.menuWrapper.find('.menu-edit-table');
    this.createTable(6, 6).appendTo(this.createMenu);
    this.createMenu.on('mouseenter', 'td', (function(_this) {
      return function(e) {
        var $td, $tr, num;
        _this.createMenu.find('td').removeClass('selected');
        $td = $(e.currentTarget);
        $tr = $td.parent();
        num = $tr.find('td').index($td) + 1;
        return $tr.prevAll('tr').addBack().find('td:lt(' + num + ')').addClass('selected');
      };
    })(this));
    this.createMenu.on('mouseleave', (function(_this) {
      return function(e) {
        return $(e.currentTarget).find('td').removeClass('selected');
      };
    })(this));
    return this.createMenu.on('mousedown', 'td', (function(_this) {
      return function(e) {
        var $closestBlock, $table, $td, $tr, colNum, rowNum;
        _this.wrapper.removeClass('menu-on');
        if (!_this.editor.inputManager.focused) {
          return;
        }
        $td = $(e.currentTarget);
        $tr = $td.parent();
        colNum = $tr.find('td').index($td) + 1;
        rowNum = $tr.prevAll('tr').length + 1;
        $table = _this.createTable(rowNum, colNum, true);
        $closestBlock = _this.editor.util.closestBlockEl();
        if (_this.editor.util.isEmptyNode($closestBlock)) {
          $closestBlock.replaceWith($table);
        } else {
          $closestBlock.after($table);
        }
        _this.decorate($table);
        _this.editor.selection.setRangeAtStartOf($table.find('td:first'));
        _this.editor.trigger('valuechanged');
        return false;
      };
    })(this));
  };

  TableButton.prototype.createTable = function(row, col, phBr) {
    var $table, $tbody, $td, $tr, c, r, _i, _j;
    $table = $('<table/>');
    $tbody = $('<tbody/>').appendTo($table);
    for (r = _i = 0; 0 <= row ? _i < row : _i > row; r = 0 <= row ? ++_i : --_i) {
      $tr = $('<tr/>').appendTo($tbody);
      for (c = _j = 0; 0 <= col ? _j < col : _j > col; c = 0 <= col ? ++_j : --_j) {
        $td = $('<td/>').appendTo($tr);
        if (phBr) {
          $td.append(this.editor.util.phBr);
        }
      }
    }
    return $table;
  };

  TableButton.prototype.refreshTableWidth = function($table) {
    var cols, tableWidth;
    tableWidth = $table.width();
    cols = $table.find('col');
    return $table.find('tr:first td').each((function(_this) {
      return function(i, td) {
        var $col;
        $col = cols.eq(i);
        return $col.attr('width', ($(td).outerWidth() / tableWidth * 100) + '%');
      };
    })(this));
  };

  TableButton.prototype.setActive = function(active) {
    TableButton.__super__.setActive.call(this, active);
    if (active) {
      this.createMenu.hide();
      return this.editMenu.show();
    } else {
      this.createMenu.show();
      return this.editMenu.hide();
    }
  };

  TableButton.prototype.deleteRow = function($td) {
    var $newTr, $tr, index;
    $tr = $td.parent('tr');
    if ($tr.siblings('tr').length < 1) {
      return this.deleteTable($td);
    } else {
      $newTr = $tr.next('tr');
      if (!($newTr.length > 0)) {
        $newTr = $tr.prev('tr');
      }
      index = $tr.find('td').index($td);
      $tr.remove();
      return this.editor.selection.setRangeAtEndOf($newTr.find('td').eq(index));
    }
  };

  TableButton.prototype.insertRow = function($td, direction) {
    var $newTr, $table, $tr, colNum, i, index, _i;
    if (direction == null) {
      direction = 'after';
    }
    $tr = $td.parent('tr');
    $table = $tr.closest('table');
    colNum = 0;
    $table.find('tr').each((function(_this) {
      return function(i, tr) {
        return colNum = Math.max(colNum, $(tr).find('td').length);
      };
    })(this));
    $newTr = $('<tr/>');
    for (i = _i = 1; 1 <= colNum ? _i <= colNum : _i >= colNum; i = 1 <= colNum ? ++_i : --_i) {
      $('<td/>').append(this.editor.util.phBr).appendTo($newTr);
    }
    $tr[direction]($newTr);
    index = $tr.find('td').index($td);
    return this.editor.selection.setRangeAtStartOf($newTr.find('td').eq(index));
  };

  TableButton.prototype.deleteCol = function($td) {
    var $newTd, $table, $tr, index;
    $tr = $td.parent('tr');
    if ($tr.siblings('tr').length < 1 && $td.siblings('td').length < 1) {
      return this.deleteTable($td);
    } else {
      index = $tr.find('td').index($td);
      $newTd = $td.next('td');
      if (!($newTd.length > 0)) {
        $newTd = $tr.prev('td');
      }
      $table = $tr.closest('table');
      $table.find('col').eq(index).remove();
      $table.find('tr').each((function(_this) {
        return function(i, tr) {
          return $(tr).find('td').eq(index).remove();
        };
      })(this));
      this.refreshTableWidth($table);
      return this.editor.selection.setRangeAtEndOf($newTd);
    }
  };

  TableButton.prototype.insertCol = function($td, direction) {
    var $col, $newCol, $newTd, $table, $tr, index, tableWidth, width;
    if (direction == null) {
      direction = 'after';
    }
    $tr = $td.parent('tr');
    index = $tr.find('td').index($td);
    $table = $td.closest('table');
    $col = $table.find('col').eq(index);
    $table.find('tr').each((function(_this) {
      return function(i, tr) {
        var $newTd;
        $newTd = $('<td/>').append(_this.editor.util.phBr);
        return $(tr).find('td').eq(index)[direction]($newTd);
      };
    })(this));
    $newCol = $('<col/>');
    $col[direction]($newCol);
    tableWidth = $table.width();
    width = Math.max(parseFloat($col.attr('width')) / 2, 50 / tableWidth * 100);
    $col.attr('width', width + '%');
    $newCol.attr('width', width + '%');
    this.refreshTableWidth($table);
    $newTd = direction === 'after' ? $td.next('td') : $td.prev('td');
    return this.editor.selection.setRangeAtStartOf($newTd);
  };

  TableButton.prototype.deleteTable = function($td) {
    var $block, $table;
    $table = $td.closest('.simditor-table');
    $block = $table.next('p');
    $table.remove();
    if ($block.length > 0) {
      return this.editor.selection.setRangeAtStartOf($block);
    }
  };

  TableButton.prototype.command = function(param) {
    var $td, range;
    range = this.editor.selection.getRange();
    $td = $(range.commonAncestorContainer).closest('td');
    if (!($td.length > 0)) {
      return;
    }
    if (param === 'deleteRow') {
      this.deleteRow($td);
    } else if (param === 'insertRowAbove') {
      this.insertRow($td, 'before');
    } else if (param === 'insertRowBelow') {
      this.insertRow($td);
    } else if (param === 'deleteCol') {
      this.deleteCol($td);
    } else if (param === 'insertColLeft') {
      this.insertCol($td, 'before');
    } else if (param === 'insertColRight') {
      this.insertCol($td);
    } else if (param === 'deleteTable') {
      this.deleteTable($td);
    } else {
      return;
    }
    return this.editor.trigger('valuechanged');
  };

  return TableButton;

})(Button);

Simditor.Toolbar.addButton(TableButton);

var StrikethroughButton,
  __hasProp = {}.hasOwnProperty,
  __extends = function(child, parent) { for (var key in parent) { if (__hasProp.call(parent, key)) child[key] = parent[key]; } function ctor() { this.constructor = child; } ctor.prototype = parent.prototype; child.prototype = new ctor(); child.__super__ = parent.prototype; return child; };

StrikethroughButton = (function(_super) {
  __extends(StrikethroughButton, _super);

  function StrikethroughButton() {
    return StrikethroughButton.__super__.constructor.apply(this, arguments);
  }

  StrikethroughButton.prototype.name = 'strikethrough';

  StrikethroughButton.prototype.icon = 'strikethrough';

  StrikethroughButton.prototype.htmlTag = 'strike';

  StrikethroughButton.prototype.disableTag = 'pre';

  StrikethroughButton.prototype.status = function($node) {
    var active;
    if ($node != null) {
      this.setDisabled($node.is(this.disableTag));
    }
    if (this.disabled) {
      return true;
    }
    active = document.queryCommandState('strikethrough') === true;
    this.setActive(active);
    return active;
  };

  StrikethroughButton.prototype.command = function() {
    document.execCommand('strikethrough');
    this.editor.trigger('valuechanged');
    return $(document).trigger('selectionchange');
  };

  return StrikethroughButton;

})(Button);

Simditor.Toolbar.addButton(StrikethroughButton);

return Simditor;

}));
