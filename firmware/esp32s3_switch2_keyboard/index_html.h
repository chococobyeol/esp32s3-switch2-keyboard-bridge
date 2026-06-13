static const char PROGMEM INDEX_HTML[] = R"rawliteral(
<!DOCTYPE html>
<html lang=en>
<head>
<meta charset="UTF-8">
<meta name = "viewport" content = "width = device-width, initial-scale = 1.0, maximum-scale = 1.0, user-scalable=0">
<title>Touch Gamepad 0.13</title>
<style type="text/css">
    table {
        display: table;
        table-layout: fixed;
        position: absolute;
        top: 0;
        bottom: 0;
        left: 0;
        right: 0;
        height: 100%;
        width: 100%;
        border-collapse: collapse;
    }

    td {
        border: 1px solid;
        font-size: 200%;
        text-align: center;
    }
</style>
<script type="text/javascript">
var websock;
var connected = false;

const NS_Control = {
  Y: 0,
  B: 1,
  A: 2,
  X: 3,
  LeftTrigger: 4,
  RightTrigger: 5,
  LeftThrottle: 6,
  RightThrottle: 7,
  Minus: 8,
  Plus: 9,
  LeftStick: 10,
  RightStick: 11,
  Home: 12,
  Capture: 13,
  Reserved1: 14,
  Reserved2: 15,
  DPadUp: 16,
  DPadDown: 17,
  DPadLeft: 18,
  DPadRight: 19,
  LeftStickUp: 20,
  LeftStickDown: 21,
  LeftStickLeft: 22,
  LeftStickRight: 23,
  RightStickUp: 24,
  RightStickDown: 25,
  RightStickLeft: 26,
  RightStickRight: 27
};

const KeyCodes = {
 "ArrowUp": NS_Control.RightStickUp,
 "ArrowDown": NS_Control.RightStickDown,
 "ArrowLeft": NS_Control.RightStickLeft,
 "ArrowRight": NS_Control.RightStickRight,
 "KeyW": NS_Control.LeftStickUp,
 "KeyS": NS_Control.LeftStickDown,
 "KeyA": NS_Control.LeftStickLeft,
 "KeyD": NS_Control.LeftStickRight,
};

const SEND_REPEAT_MS = 20;
const MIN_PRESS_MS = 90;
const releaseTimers = new Map();
const repeatTimers = new Map();
const pressStartedAt = new Map();

function wsSend(json) {
  if (connected && websock && websock.readyState === WebSocket.OPEN) {
    websock.send(json);
  }
}

function startRepeating(id, json) {
  clearTimeout(releaseTimers.get(id));
  releaseTimers.delete(id);
  if (repeatTimers.has(id)) return;
  pressStartedAt.set(id, performance.now());
  wsSend(json);
  repeatTimers.set(id, setInterval(function() { wsSend(json); }, SEND_REPEAT_MS));
}

function stopRepeating(id, releaseJson) {
  const started = pressStartedAt.get(id) || performance.now();
  const elapsed = performance.now() - started;
  const delay = Math.max(0, MIN_PRESS_MS - elapsed);
  clearInterval(repeatTimers.get(id));
  repeatTimers.delete(id);
  pressStartedAt.delete(id);
  clearTimeout(releaseTimers.get(id));
  releaseTimers.set(id, setTimeout(function() {
    wsSend(releaseJson);
    setTimeout(function() { wsSend(releaseJson); }, SEND_REPEAT_MS);
    setTimeout(function() { wsSend(releaseJson); }, SEND_REPEAT_MS * 2);
    releaseTimers.delete(id);
  }, delay));
}

document.addEventListener('keydown', function(event) {
  if (event.code in KeyCodes) {
    event.preventDefault();
    const id = 'key:' + event.code;
    startRepeating(id, JSON.stringify({event:'keydown', code:KeyCodes[event.code]}));
  }
}, false);

document.addEventListener('keyup', function(event) {
  if (event.code in KeyCodes) {
    event.preventDefault();
    const id = 'key:' + event.code;
    stopRepeating(id, JSON.stringify({event:'keyup', code:KeyCodes[event.code]}));
  }
}, false);

function touch_start(event) {
    event.preventDefault();
    let e = document.getElementById(this.id);
    e.style.backgroundColor = "yellow";
    var json = JSON.stringify({event:'touch start', name:this.innerHTML, row:this.row, col:this.col});
    if (!connected) {
        window.location.reload();
    }
    else {
        startRepeating('cell:' + this.id, json);
    }
}
function touch_end(event) {
    event.preventDefault();
    let e = document.getElementById(this.id);
    e.style.backgroundColor = "white";
    stopRepeating('cell:' + this.id, JSON.stringify({event:'touch end', name:this.innerHTML, id:this.id, row:this.row, col:this.col}));
}
function touch_move(event) {
    event.preventDefault();
}
function touch_cancel(event) {
    event.preventDefault();
    document.getElementById(this.id).style.backgroundColor = "white";
    stopRepeating('cell:' + this.id, JSON.stringify({event:'touch cancel', name:this.innerHTML, id:this.id, row:this.row, col:this.col}));
}
function button_click(event) {
    event.preventDefault();
}
function double_click(event) {
    event.preventDefault();
}
function context_menu(event) {
    event.preventDefault();
}
function generateGrid(cellData, id) {
    let grid = document.getElementById(id);
    let r = 0;
    for (let rowElement of cellData) {
        let aRow = grid.insertRow();
        let c = 0;
        for (let colElement of rowElement) {
            let cell = aRow.insertCell();
            cell.id = 'r' + r + 'c' + c;
            cell['row'] = r;
            cell['col'] = c;
            cell.onclick = button_click;
            cell.ondblclick = double_click;
            cell.oncontextmenu = context_menu;
            cell.ontouchstart = touch_start;
            cell.onmousedown = touch_start;
            cell.ontouchmove = touch_move;
            cell.onmousemove = touch_move;
            cell.ontouchend = touch_end;
            cell.onmouseup = touch_end;
            cell.onmouseout = touch_end;
            cell.onmouseleave = touch_end;
            cell.ontouchcancel = touch_cancel;
            cell.innerHTML = colElement["cellLabel"];
            if (colElement["colSpan"] > 1) {
                cell.colSpan = colElement["colSpan"];
            }
            c++;
        }
        r++;
    }
}

var FullPage = document.documentElement;
function openFullscreen() {
    if (FullPage.requestFullscreen) {
        FullPage.requestFullscreen();
    } else if (FullPage.mozRequestFullScreen) { /* Firefox */
        FullPage.mozRequestFullScreen();
    } else if (FullPage.webkitRequestFullscreen) { /* Chrome, Safari & Opera */
        FullPage.webkitRequestFullscreen();
    } else if (FullPage.msRequestFullscreen) { /* IE/Edge */
        FullPage.msRequestFullscreen();
    }
}

function start() {
    websock = new WebSocket('ws://' + window.location.hostname + ':81/');
    websock.onopen = function(evt) { console.log('websock onopen', evt); connected = true; };
    websock.onclose = function(evt) { console.log('websock onclose', evt); connected = false; };
    websock.onerror = function(evt) { console.log('websock onerror', evt); };
    websock.onmessage = function(evt) {
        generateGrid(JSON.parse(evt.data), "my_table");
    };
}

</script>
</head>
<body onload="javascript:start();" onresize="window.location.reload();">
<table id="my_table">
<!-- Grid goes here -->
</table>
</body>
</html>
)rawliteral";
