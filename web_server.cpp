#include "web_server.h"
#include "macro_store.h"
#include "ui.h"
#include "config.h"
#include "energy_save.h"
#include <WiFi.h>
#include <WebServer.h>
#include <Preferences.h>
#include <LittleFS.h>
#include "esp_system.h"
#include "soc/rtc_cntl_reg.h"

// ── HTTP server instance ──────────────────────────────────────────────────────
static WebServer server(WEB_SERVER_PORT);

// ── Embedded HTML page ────────────────────────────────────────────────────────
static const char HTML_PAGE[] = R"rawliteral(<!DOCTYPE html>
<html lang="en">
<head>
<meta charset="UTF-8">
<meta name="viewport" content="width=device-width,initial-scale=1">
<title>m5Macro</title>
<style>
:root{--clr-bg:#0a0a0a;--clr-surface:#111111;--clr-border:#222222;--clr-text:#e0e0e0;--clr-dim:#888888;--clr-accent:#888888}
*{box-sizing:border-box;margin:0;padding:0}
body{background:var(--clr-bg);color:var(--clr-text);font-family:'Courier New',monospace;font-size:13px;display:flex;flex-direction:column;height:100vh;overflow:hidden}
header{background:var(--clr-surface);padding:10px 14px;display:flex;align-items:center;gap:10px;border-bottom:1px solid var(--clr-border);flex-shrink:0}
h1{color:#ffffff;font-size:16px;letter-spacing:2px}
.sub{color:#444444;font-size:11px}
.conn{width:7px;height:7px;border-radius:50%;background:var(--clr-dim);flex-shrink:0;transition:background .3s}
.conn.off{background:#444444}
.ip-badge{color:#cccccc;font-size:11px;margin-left:auto;background:#1a1a1a;padding:3px 8px;border-radius:3px;border:1px solid #333333}
.hbg{display:none;flex-direction:column;gap:4px;background:none;border:1px solid #222222;border-radius:4px;padding:6px 7px;cursor:pointer;flex-shrink:0}
.hbg span{display:block;width:18px;height:2px;background:#606060;border-radius:1px;transition:background .1s}
.hbg:hover span{background:#a0a0a0}
.settings-btn{background:none;border:1px solid #222222;border-radius:4px;color:#888888;cursor:pointer;font-size:16px;padding:5px 9px;line-height:1;flex-shrink:0}
.settings-btn:hover{color:#e0e0e0;border-color:#444444}
/* Settings Modal */
#settings-modal{display:none;position:fixed;inset:0;background:rgba(0,0,0,.85);z-index:200;align-items:flex-start;justify-content:center;overflow-y:auto;padding:4vh 8px 20px}
#settings-modal.show{display:flex}
.modal-box{background:var(--clr-surface);border:1px solid #2a2a2a;border-radius:8px;width:340px;max-width:96vw;display:flex;flex-direction:column;overflow:hidden}
.modal-hdr{display:flex;align-items:center;justify-content:space-between;padding:14px 16px 13px;border-bottom:1px solid #1e1e1e}
.modal-ttl{color:#ffffff;font-size:14px;letter-spacing:1px}
.modal-x{background:none;border:none;color:#555555;font-size:18px;cursor:pointer;padding:0 4px;line-height:1}
.modal-x:hover{color:#e0e0e0}
.s-sect{border-bottom:1px solid #1a1a1a}
.s-sect:last-of-type{border-bottom:none}
.s-hdr{width:100%;background:none;border:none;color:#666666;padding:11px 16px;display:flex;align-items:center;gap:9px;cursor:pointer;font-family:inherit;font-size:11px;text-align:left;letter-spacing:.5px;transition:background .12s,color .12s}
.s-hdr:hover{background:#161616;color:#cccccc}
.s-hdr.open{color:#dddddd;background:#141414}
.s-ttl{flex:1;font-weight:bold;text-transform:uppercase}
.s-chev{color:#333333;font-size:9px;transition:transform .18s}
.s-hdr.open .s-chev{transform:rotate(90deg);color:#666666}
.s-body{padding:14px 16px;display:flex;flex-direction:column;gap:11px;border-top:1px solid #1a1a1a}
.s-body.closed{display:none}
.s-danger .s-hdr{color:#663333}
.s-danger .s-hdr:hover{background:#160808;color:#cc6060}
.s-danger .s-hdr.open{color:#cc6060;background:#130606}
.modal-fg{display:flex;flex-direction:column;gap:5px}
.modal-lbl{color:#555555;font-size:10px;text-transform:uppercase;letter-spacing:1px}
.modal-inp{background:var(--clr-bg);border:1px solid #2e2e2e;color:var(--clr-text);padding:9px 10px;border-radius:4px;width:100%;font-family:inherit;font-size:13px}
.modal-inp:focus{outline:none;border-color:#666666}
.btn-s{padding:9px;background:#1e1e1e;color:#aaaaaa;border:1px solid #2e2e2e;border-radius:4px;cursor:pointer;font-family:inherit;font-size:11px;font-weight:bold;text-transform:uppercase;letter-spacing:.5px;width:100%;transition:background .12s}
.btn-s:hover{background:#282828;color:#e0e0e0;border-color:#444444}
.btn-s.prim{background:#1a2e1a;border-color:#2a4a2a;color:#7ac07a}
.btn-s.prim:hover{background:#1f3a1f;border-color:#3a6a3a;color:#99d099}
.modal-row{display:flex;gap:8px}
.btn-restart{flex:1;padding:9px;background:#181818;color:#aaaaaa;border:1px solid #2a2a2a;border-radius:4px;cursor:pointer;font-family:inherit;font-size:11px;font-weight:bold;text-transform:uppercase;letter-spacing:.5px}
.btn-restart:hover{background:#222222;color:#ffffff;border-color:#555555}
.btn-boot{flex:1;padding:9px;background:#1a0808;color:#884444;border:1px solid #2a1414;border-radius:4px;cursor:pointer;font-family:inherit;font-size:11px;font-weight:bold;text-transform:uppercase;letter-spacing:.5px}
.btn-boot:hover{background:#220a0a;color:#cc6060;border-color:#441515}
.modal-st{font-size:11px;min-height:30px;padding:7px 16px;color:#666666;text-align:center;border-top:1px solid #1a1a1a;display:flex;align-items:center;justify-content:center}
.modal-st.ok{color:#66aa66}
.modal-st.err{color:#cc5555}
/* Overlay */
#ov{display:none;position:fixed;inset:0;background:rgba(0,0,0,.65);z-index:90}
#ov.show{display:block}
/* Layout */
.layout{display:flex;flex:1;overflow:hidden}
/* Sidebar */
.sidebar{width:190px;background:#0d0d0d;border-right:1px solid var(--clr-border);display:flex;flex-direction:column;flex-shrink:0;transition:transform .22s ease}
.sb-hdr{display:flex;align-items:center;gap:6px;padding:11px 10px 5px;border-bottom:1px solid #1a1a1a}
.sb-ttl{color:#444444;font-size:10px;text-transform:uppercase;letter-spacing:1.5px;flex:1}
.mc{background:#1a1a1a;color:#555555;font-size:10px;padding:1px 6px;border-radius:10px}
.sb-x{display:none;background:none;border:none;color:#555555;font-size:18px;cursor:pointer;padding:0 4px;line-height:1}
.sb-x:hover{color:#a0a0a0}
.macro-list{flex:1;overflow-y:auto;padding:4px 4px 0;-webkit-overflow-scrolling:touch}
.mi{padding:10px;border-radius:4px;cursor:pointer;color:#666666;font-size:12px;white-space:nowrap;overflow:hidden;text-overflow:ellipsis;border-left:2px solid transparent;display:flex;align-items:center;gap:5px;min-height:44px;transition:background .1s,color .1s}
.mi:hover{background:#151515;color:#aaaaaa}
.mi.active{background:#1a1a1a;color:var(--clr-text);border-left-color:var(--clr-accent)}
.mi-num{color:#333333;font-size:10px;min-width:14px;text-align:right;flex-shrink:0}
.mi-name{overflow:hidden;text-overflow:ellipsis;flex:1}
.ud{color:#aaaaaa;font-size:8px;flex-shrink:0;opacity:0}
.ud.show{opacity:1}
.sb-foot{padding:8px}
/* Main */
.main{flex:1;display:flex;flex-direction:column;overflow:hidden;min-width:0}
.editor{flex:1;padding:14px 16px;display:flex;flex-direction:column;gap:10px;overflow-y:auto;-webkit-overflow-scrolling:touch}
.ph{display:flex;flex-direction:column;align-items:center;justify-content:center;flex:1;gap:8px;color:#222222}
.ph-i{font-size:26px}.ph-t{font-size:12px}
/* Form */
.fg{display:flex;flex-direction:column;gap:5px}
.fh{display:flex;align-items:center;justify-content:space-between}
label{color:#555555;font-size:10px;text-transform:uppercase;letter-spacing:1px}
.sc{color:#333333;font-size:10px}
input[type=text]{background:#0d0d0d;border:1px solid var(--clr-border);color:var(--clr-text);padding:9px 10px;border-radius:4px;width:100%;font-family:inherit;font-size:16px;transition:border-color .15s;-webkit-appearance:none}
input[type=text]:focus{outline:none;border-color:#888888}
/* Step rows */
.step-list{display:flex;flex-direction:column;gap:5px}
.sr{display:flex;flex-wrap:wrap;align-items:center;gap:6px;background:#0d0d0d;border:1px solid #1a1a1a;border-radius:5px;padding:8px 9px}
.sr:hover{border-color:#2a2a2a}
.type-sel{flex:1;min-width:0}
.sr-acts{display:flex;gap:4px;margin-left:auto;flex-shrink:0}
.sr-cnt{width:100%;display:flex;align-items:center;gap:6px;flex-wrap:wrap}
.si{background:#0e0e0e;border:1px solid #222222;color:#cccccc;padding:9px 10px;border-radius:4px;font-family:inherit;font-size:16px;cursor:pointer;min-height:44px;-webkit-appearance:none}
.si:focus{outline:none;border-color:#888888}
.key-sel{flex:1;min-width:0}
.text-in{flex:1;min-width:0;background:#0e0e0e;border:1px solid #222222;color:#e0e0e0;padding:9px 10px;border-radius:4px;font-family:inherit;font-size:16px;min-height:44px;-webkit-appearance:none}
.text-in:focus{outline:none;border-color:#888888}
.delay-in{width:90px;background:#0e0e0e;border:1px solid #222222;color:#cccccc;padding:9px 10px;border-radius:4px;font-family:inherit;font-size:16px;min-height:44px;-webkit-appearance:none}
.delay-in:focus{outline:none;border-color:#888888}
input[type=number]::-webkit-outer-spin-button,input[type=number]::-webkit-inner-spin-button{-webkit-appearance:none}
input[type=number]{-moz-appearance:textfield}
.ms-lbl{color:#444444;font-size:12px;flex-shrink:0}
.mods{display:flex;gap:4px;flex-wrap:wrap}
.mb{padding:9px 12px;background:#141414;border:1px solid #222222;color:#555555;border-radius:4px;cursor:pointer;font-size:13px;font-family:inherit;user-select:none;min-height:44px;display:flex;align-items:center;transition:background .1s,color .1s,border-color .1s}
.mb.on{background:#1a1a1a;color:#e0e0e0;border-color:#555555}
.plus-lbl{color:#444444;font-size:15px;flex-shrink:0}
.ob{width:40px;height:40px;font-size:13px;background:#141414;border:1px solid #222222;color:#444444;border-radius:4px;cursor:pointer;display:flex;align-items:center;justify-content:center;flex-shrink:0;font-family:inherit}
.ob:hover:not([disabled]){color:#888888;border-color:#333333;background:#1a1a1a}
.ob[disabled]{opacity:.3;cursor:default}
.rm{width:44px;height:40px;background:#1a0a0a;border:1px solid #2a1010;color:#664444;border-radius:4px;cursor:pointer;font-size:16px;display:flex;align-items:center;justify-content:center;flex-shrink:0}
.rm:hover{color:#cc6060;border-color:#441515;background:#220a0a}
.add-btn{padding:10px 16px;background:#0f0f0f;border:1px solid #222222;color:#444444;border-radius:5px;cursor:pointer;font-family:inherit;font-size:13px;margin-top:2px;min-height:44px;width:100%;text-align:center}
.add-btn:hover{color:#888888;border-color:#333333}
.toolbar{display:flex;gap:8px;align-items:center;flex-wrap:wrap;padding-top:4px}
.btn-save{padding:10px 18px;background:#333333;color:#ffffff;border:none;border-radius:5px;cursor:pointer;font-family:inherit;font-size:13px;font-weight:bold;letter-spacing:.5px;text-transform:uppercase;min-height:44px}
.btn-save:hover{background:#444444}
.btn-del{padding:10px 16px;background:#1a0a0a;color:#aa4444;border:1px solid #2a1010;border-radius:5px;cursor:pointer;font-family:inherit;font-size:13px;font-weight:bold;letter-spacing:.5px;text-transform:uppercase;min-height:44px}
.btn-del:hover{color:#dd6060}
.btn-new{padding:10px 16px;background:#141414;color:#666666;border:1px solid #222222;border-radius:5px;cursor:pointer;font-family:inherit;font-size:13px;font-weight:bold;letter-spacing:.5px;text-transform:uppercase;min-height:44px}
.btn-new:hover{color:#aaaaaa}
.kh{font-size:10px;color:#222222;margin-left:auto}
.status{font-size:11px;min-height:16px;padding:2px 0;transition:opacity .5s}
::-webkit-scrollbar{width:4px}::-webkit-scrollbar-track{background:#0a0a0a}::-webkit-scrollbar-thumb{background:#222222;border-radius:2px}
.es-sw{position:relative;display:inline-block;width:36px;height:20px;flex-shrink:0}
.es-sw input{opacity:0;width:0;height:0}
.es-sl{position:absolute;cursor:pointer;inset:0;background:#2a2a2a;border-radius:20px;transition:.2s}
.es-sl:before{content:"";position:absolute;width:14px;height:14px;left:3px;bottom:3px;background:#555;border-radius:50%;transition:.2s}
input:checked+.es-sl{background:#444}
input:checked+.es-sl:before{transform:translateX(16px);background:#e0e0e0}
.es-row{display:flex;align-items:center;gap:8px}
.es-rng{flex:1;accent-color:#888;cursor:pointer}
.es-val{color:#666;font-size:11px;min-width:26px;text-align:right;flex-shrink:0}
.clr-grid{display:flex;flex-direction:column;gap:6px}
.clr-row{display:flex;align-items:center;justify-content:space-between;gap:8px}
.clr-lbl{color:#666666;font-size:11px;flex:1}
.clr-inp{width:44px;height:28px;padding:2px 3px;background:var(--clr-bg);border:1px solid #333333;border-radius:4px;cursor:pointer}
@media(min-width:600px){
  .hbg{display:none}
  .sb-x{display:none!important}
  .sub{display:inline}
  .sr{flex-wrap:nowrap}
  .type-sel{width:95px;flex:0 0 95px}
  .sr-acts{margin-left:0}
  .sr-cnt{width:auto;flex:1}
  .si,.text-in,.delay-in{font-size:13px;min-height:34px;padding:5px 8px}
  input[type=text]{font-size:13px;padding:8px 10px}
  .ob{width:30px;height:30px;font-size:11px}
  .rm{width:34px;height:30px;font-size:13px}
  .mb{min-height:32px;padding:4px 9px;font-size:11px}
  .btn-save,.btn-del,.btn-new{min-height:34px;font-size:11px;padding:6px 14px}
  .add-btn{min-height:34px;font-size:11px;padding:6px 12px;width:auto}
  .mi{min-height:32px;padding:7px 10px}
}
@media(max-width:599px){
  .hbg{display:flex}
  .sub{display:none}
  .sb-x{display:block}
  .ip-badge{font-size:10px;padding:3px 6px}
  .sidebar{position:fixed;top:0;left:0;bottom:0;z-index:100;transform:translateX(-100%);width:220px;box-shadow:4px 0 24px rgba(0,0,0,.8)}
  .sidebar.open{transform:translateX(0)}
  .editor{padding:12px}
}
</style>
</head>
<body>
<div id="ov" onclick="closeSB()"></div>
<div id="settings-modal">
  <div class="modal-box">
    <div class="modal-hdr">
      <span class="modal-ttl">&#9881; Settings</span>
      <button class="modal-x" onclick="closeSettings()">&#10005;</button>
    </div>
    <div class="s-sect">
      <button class="s-hdr open" id="sh-wifi" onclick="togSect('sb-wifi','sh-wifi')">
        <span>&#128246;</span><span class="s-ttl">WiFi</span><span class="s-chev">&#9654;</span>
      </button>
      <div class="s-body" id="sb-wifi">
        <div class="modal-fg"><label class="modal-lbl">SSID</label><input type="text" class="modal-inp" id="cfg-ssid" maxlength="32" placeholder="m5Macro" autocomplete="off"></div>
        <div class="modal-fg"><label class="modal-lbl">Password (min. 8 chars)</label><input type="password" class="modal-inp" id="cfg-pass" maxlength="63" placeholder="(leave unchanged)" autocomplete="new-password"></div>
        <button class="btn-s prim" onclick="saveSettings()">&#10003; Save &amp; Restart</button>
      </div>
    </div>
    <div class="s-sect">
      <button class="s-hdr" id="sh-energy" onclick="togSect('sb-energy','sh-energy')">
        <span>&#9889;</span><span class="s-ttl">Energy Saving</span><span class="s-chev">&#9654;</span>
      </button>
      <div class="s-body closed" id="sb-energy">
        <div class="modal-fg">
          <div style="display:flex;align-items:center;justify-content:space-between">
            <label class="modal-lbl" style="margin:0">Enable</label>
            <label class="es-sw"><input type="checkbox" id="cfg-es-en"><span class="es-sl"></span></label>
          </div>
        </div>
        <div class="modal-fg"><label class="modal-lbl">Inactivity until dim (seconds)</label><input type="number" class="modal-inp" id="cfg-es-to" min="5" max="3600" value="30"></div>
        <div class="modal-fg"><label class="modal-lbl">Dim brightness (0–255)</label><div class="es-row"><input type="range" class="es-rng" id="cfg-es-db" min="0" max="255" value="10" oninput="document.getElementById('cfg-es-dbv').textContent=this.value"><span class="es-val" id="cfg-es-dbv">10</span></div></div>
        <div class="modal-fg"><label class="modal-lbl">Normal brightness (10–255)</label><div class="es-row"><input type="range" class="es-rng" id="cfg-es-ab" min="10" max="255" value="128" oninput="document.getElementById('cfg-es-abv').textContent=this.value"><span class="es-val" id="cfg-es-abv">128</span></div></div>
        <button class="btn-s" onclick="saveEnergy()">&#9889; Save energy settings</button>
      </div>
    </div>
    <div class="s-sect">
      <button class="s-hdr" id="sh-fwclr" onclick="togSect('sb-fwclr','sh-fwclr')">
        <span>&#9632;</span><span class="s-ttl">Display Colors</span><span class="s-chev">&#9654;</span>
      </button>
      <div class="s-body closed" id="sb-fwclr">
        <div class="clr-grid">
          <div class="clr-row"><span class="clr-lbl">Background</span><input type="color" class="clr-inp" id="fw-bg" value="#000000"></div>
          <div class="clr-row"><span class="clr-lbl">Roller surface</span><input type="color" class="clr-inp" id="fw-surface" value="#000000"></div>
          <div class="clr-row"><span class="clr-lbl">Accent (border)</span><input type="color" class="clr-inp" id="fw-accent" value="#808080"></div>
          <div class="clr-row"><span class="clr-lbl">Selected background</span><input type="color" class="clr-inp" id="fw-sel-bg" value="#1a1a1a"></div>
          <div class="clr-row"><span class="clr-lbl">Active text</span><input type="color" class="clr-inp" id="fw-text" value="#ffffff"></div>
          <div class="clr-row"><span class="clr-lbl">Inactive text</span><input type="color" class="clr-inp" id="fw-text-dim" value="#888888"></div>
        </div>
        <button class="btn-s" onclick="saveFwColors()">&#9632; Save display colors</button>
      </div>
    </div>
    <button class="btn-modal-save" onclick="saveEnergy()" style="width:100%">&#9889; Save energy settings</button>
    <hr class="modal-sep">
    <div class="modal-fg">
      <label class="modal-lbl">&#127916; Screensaver GIF</label>
    </div>
    <div class="modal-fg">
      <div style="display:flex;align-items:center;justify-content:space-between">
        <label class="modal-lbl" style="margin:0">Play GIF on inactivity</label>
        <label class="es-sw"><input type="checkbox" id="cfg-ss-gif"><span class="es-sl"></span></label>
      </div>
    </div>
    <div class="modal-fg">
      <div id="ss-gif-st" style="color:#666666;font-size:11px;margin-bottom:6px">No GIF uploaded</div>
      <input type="file" id="ss-gif-file" accept=".gif,image/gif" class="modal-inp" style="padding:7px;cursor:pointer">
    </div>
    <div class="modal-row">
      <button class="btn-modal-save" onclick="uploadGif()" style="flex:1">&#8679; Upload GIF</button>
      <button class="btn-restart" onclick="deleteGif()" style="flex:1">&#10007; Delete GIF</button>
    </div>
    <hr class="modal-sep">
    <div class="modal-fg">
      <label class="modal-lbl">&#9632; Display Colors (Firmware)</label>
    </div>
    <div class="clr-grid">
      <div class="clr-row"><span class="clr-lbl">Background</span><input type="color" class="clr-inp" id="fw-bg" value="#000000"></div>
      <div class="clr-row"><span class="clr-lbl">Roller surface</span><input type="color" class="clr-inp" id="fw-surface" value="#000000"></div>
      <div class="clr-row"><span class="clr-lbl">Accent (border)</span><input type="color" class="clr-inp" id="fw-accent" value="#808080"></div>
      <div class="clr-row"><span class="clr-lbl">Selected background</span><input type="color" class="clr-inp" id="fw-sel-bg" value="#1a1a1a"></div>
      <div class="clr-row"><span class="clr-lbl">Active text</span><input type="color" class="clr-inp" id="fw-text" value="#ffffff"></div>
      <div class="clr-row"><span class="clr-lbl">Inactive text</span><input type="color" class="clr-inp" id="fw-text-dim" value="#888888"></div>
    </div>
    <button class="btn-modal-save" onclick="saveFwColors()" style="width:100%">&#9632; Save display colors</button>
    <hr class="modal-sep">
    <div class="modal-fg">
      <label class="modal-lbl">&#9632; Web UI Colors</label>
    </div>
    <div class="s-sect s-danger">
      <button class="s-hdr" id="sh-dev" onclick="togSect('sb-dev','sh-dev')">
        <span>&#9888;</span><span class="s-ttl">Device</span><span class="s-chev">&#9654;</span>
      </button>
      <div class="s-body closed" id="sb-dev">
        <div class="modal-row">
          <button class="btn-restart" onclick="doRestart()">&#8635; Restart</button>
          <button class="btn-boot" onclick="doBootloader()">&#9660; Bootloader</button>
        </div>
      </div>
    </div>
    <div class="modal-st" id="modal-st"></div>
  </div>
</div>
<header>
  <button class="hbg" onclick="openSB()" aria-label="Menu"><span></span><span></span><span></span></button>
  <div class="conn" id="conn"></div>
  <h1>m5Macro</h1><span class="sub">Editor</span>
  <span class="ip-badge" id="ip-badge">192.168.4.1</span>
  <button class="settings-btn" onclick="openSettings()" title="Settings">&#9881;</button>
</header>
<div class="layout">
  <div class="sidebar" id="sb">
    <div class="sb-hdr">
      <span class="sb-ttl">Macros</span>
      <span class="mc" id="mc">0</span>
      <button class="sb-x" onclick="closeSB()">&#10005;</button>
    </div>
    <div class="macro-list" id="ml"></div>
    <div class="sb-foot">
      <button class="btn-new" onclick="newMacro()" style="width:100%;text-align:center">+ New</button>
    </div>
  </div>
  <div class="main">
    <div class="editor" id="editor">
      <div class="ph"><div class="ph-i">&#9000;</div><div class="ph-t">Select a macro or create a new one</div></div>
    </div>
  </div>
</div>
<script>
var macros=[],cur=-1,dirty=false,steps=[],stTimer=null;
var KEYS=['ENTER','ESC','BACKSPACE','TAB','SPACE','DELETE','INSERT','UP','DOWN','LEFT','RIGHT','HOME','END','PGUP','PGDN','F1','F2','F3','F4','F5','F6','F7','F8','F9','F10','F11','F12','CAPSLOCK','PRINTSCREEN','SCROLLLOCK','PAUSE','NUMLOCK','A','B','C','D','E','F','G','H','I','J','K','L','M','N','O','P','Q','R','S','T','U','V','W','X','Y','Z','0','1','2','3','4','5','6','7','8','9'];
var MODS=['CTRL','SHIFT','ALT','WIN'];

function esc(s){return(s||'').replace(/&/g,'&amp;').replace(/</g,'&lt;').replace(/>/g,'&gt;').replace(/"/g,'&quot;');}
function setOnline(on){document.getElementById('conn').classList.toggle('off',!on);}
function openSB(){document.getElementById('sb').classList.add('open');document.getElementById('ov').classList.add('show');}
function closeSB(){document.getElementById('sb').classList.remove('open');document.getElementById('ov').classList.remove('show');}

function openSettings(){
  Promise.all([
    fetch('/api/settings').then(function(r){return r.json();}).catch(function(){return{};}),
    fetch('/api/energy').then(function(r){return r.json();}).catch(function(){return{};}),
    fetch('/api/colors').then(function(r){return r.json();}).catch(function(){return{};}),
    fetch('/api/webcolors').then(function(r){return r.json();}).catch(function(){return{};}),
    fetch('/api/screensaver').then(function(r){return r.json();}).catch(function(){return{};})
  ]).then(function(res){
    var j=res[0],e=res[1],fc=res[2],wc=res[3],ss=res[4];
    document.getElementById('cfg-ssid').value=j.ssid||'';
    document.getElementById('cfg-pass').value='';
    document.getElementById('modal-st').textContent='';
    if(e.enabled!==undefined)document.getElementById('cfg-es-en').checked=!!e.enabled;
    if(e.timeout_s!==undefined)document.getElementById('cfg-es-to').value=e.timeout_s;
    if(e.dim_br!==undefined){document.getElementById('cfg-es-db').value=e.dim_br;document.getElementById('cfg-es-dbv').textContent=e.dim_br;}
    if(e.active_br!==undefined){document.getElementById('cfg-es-ab').value=e.active_br;document.getElementById('cfg-es-abv').textContent=e.active_br;}
    if(ss.gif_mode!==undefined)document.getElementById('cfg-ss-gif').checked=!!ss.gif_mode;
    var ssst=document.getElementById('ss-gif-st');
    if(ssst){if(ss.has_gif){ssst.textContent='GIF: hochgeladen \u2713';ssst.style.color='#aaaaaa';}else{ssst.textContent='Kein GIF hochgeladen';ssst.style.color='#666666';}}
    if(fc.bg!==undefined){
      document.getElementById('fw-bg').value='#'+fc.bg;
      document.getElementById('fw-surface').value='#'+fc.surface;
      document.getElementById('fw-accent').value='#'+fc.accent;
      document.getElementById('fw-sel-bg').value='#'+fc.sel_bg;
      document.getElementById('fw-text').value='#'+fc.text;
      document.getElementById('fw-text-dim').value='#'+fc.text_dim;
    }
    if(wc.bg!==undefined){
      document.getElementById('web-bg').value='#'+wc.bg;
      document.getElementById('web-surface').value='#'+wc.surface;
      document.getElementById('web-border').value='#'+wc.border;
      document.getElementById('web-text').value='#'+wc.text;
      document.getElementById('web-dim').value='#'+wc.dim;
      document.getElementById('web-accent').value='#'+wc.accent;
    }
    document.getElementById('settings-modal').classList.add('show');
  });
}
function closeSettings(){document.getElementById('settings-modal').classList.remove('show');}
function togSect(bodyId,hdrId){var b=document.getElementById(bodyId);var h=document.getElementById(hdrId);var closing=!b.classList.contains('closed');b.classList.toggle('closed',closing);h.classList.toggle('open',!closing);}

function mst(msg,cls){var el=document.getElementById('modal-st');el.textContent=msg;el.className='modal-st'+(cls?' '+cls:'');}

async function saveSettings(){
  var ssid=(document.getElementById('cfg-ssid').value||'').trim();
  var pass=document.getElementById('cfg-pass').value||'';
  if(!ssid){mst('SSID cannot be empty.');return;}
  if(pass.length>0&&pass.length<8){mst('Password must be at least 8 characters.');return;}
  mst('Saving...');
  var body=new URLSearchParams({ssid:ssid,pass:pass});
  try{
    var r=await fetch('/api/settings',{method:'POST',body:body});
    var j=await r.json();
    if(j.ok){mst('Saved! Device is restarting...','ok');}
    else mst('Error: '+(j.err||'?'),'err');
  }catch(e){mst('Connection lost – restarting.','err');}
}

async function doRestart(){
  if(!confirm('Restart device now?'))return;
  mst('Restarting...');
  try{await fetch('/api/restart',{method:'POST'});}catch(e){}
  mst('Device is restarting...');
}

async function doBootloader(){
  if(!confirm('Switch to bootloader mode?'))return;
  mst('Bootloader...');
  try{await fetch('/api/restart-bootloader',{method:'POST'});}catch(e){}
  mst('Device is switching to bootloader mode...');
}

function parseSteps(raw){
  var out=[];
  (raw||'').split('\n').forEach(function(ln){
    ln=ln.trim();
    if(!ln||ln[0]==='#')return;
    if(ln.indexOf('TEXT:')=== 0){out.push({type:'text',text:ln.substring(5)});return;}
    if(ln.indexOf('DELAY:')=== 0){out.push({type:'delay',ms:parseInt(ln.substring(6))||100});return;}
    var p=ln.split('+');
    if(p.length>1){
      var mods=p.slice(0,-1).map(function(m){
        m=m.trim().toUpperCase();
        if(m==='LCTRL'||m==='RCTRL')return'CTRL';
        if(m==='LSHIFT'||m==='RSHIFT')return'SHIFT';
        if(m==='LALT'||m==='RALT')return'ALT';
        if(m==='RWIN'||m==='GUI')return'WIN';
        return m;
      }).filter(function(m){return MODS.indexOf(m)>=0;});
      if(mods.length){out.push({type:'combo',mods:mods,key:p[p.length-1].trim().toUpperCase()});return;}
    }
    out.push({type:'key',key:ln.toUpperCase()});
  });
  return out;
}

function serializeSteps(arr){
  return arr.map(function(s){
    if(s.type==='text')return'TEXT:'+s.text;
    if(s.type==='delay')return'DELAY:'+s.ms;
    if(s.type==='combo')return(s.mods.length?s.mods.join('+')+'+':'')+s.key;
    return s.key;
  }).join('\n');
}

function kOpts(sel){
  var list=KEYS.slice();
  if(sel&&list.indexOf(sel)<0)list.unshift(sel);
  return list.map(function(k){return'<option value="'+k+'"'+(k===sel?' selected':'')+'>'+k+'</option>';}).join('');
}

function renderRow(s,i){
  var n=steps.length;
  var r='<div class="sr">'
    +'<select class="si type-sel" onchange="chType('+i+',this.value)">'
    +'<option value="key"'+(s.type==='key'?' selected':'')+'>Key</option>'
    +'<option value="combo"'+(s.type==='combo'?' selected':'')+'>Combo</option>'
    +'<option value="text"'+(s.type==='text'?' selected':'')+'>Text</option>'
    +'<option value="delay"'+(s.type==='delay'?' selected':'')+'>Delay</option>'
    +'</select>'
    +'<div class="sr-acts">'
    +'<button class="ob" onclick="mv('+i+',-1)"'+(i===0?' disabled':'')+' title="Up">&#9650;</button>'
    +'<button class="ob" onclick="mv('+i+',1)"'+(i===n-1?' disabled':'')+' title="Down">&#9660;</button>'
    +'<button class="rm" onclick="rmStep('+i+')" title="Remove">&#10007;</button>'
    +'</div>'
    +'<div class="sr-cnt">';
  if(s.type==='key'){
    r+='<select class="si key-sel" onchange="steps['+i+'].key=this.value;markDirty()">'+kOpts(s.key||'ENTER')+'</select>';
  }else if(s.type==='combo'){
    r+='<div class="mods">';
    MODS.forEach(function(m){r+='<button class="mb'+(s.mods&&s.mods.indexOf(m)>=0?' on':'')+'" onclick="togMod('+i+',\''+m+'\')">'+m+'</button>';});
    r+='</div><span class="plus-lbl">+</span><select class="si" style="min-width:80px" onchange="steps['+i+'].key=this.value;markDirty()">'+kOpts(s.key||'C')+'</select>';
  }else if(s.type==='text'){
    r+='<input type="text" class="text-in" placeholder="Enter text..." value="'+esc(s.text||'')+'" oninput="steps['+i+'].text=this.value;markDirty()" autocomplete="off">';
  }else{
    r+='<input type="number" class="delay-in" min="1" max="9999" value="'+(s.ms||100)+'" oninput="steps['+i+'].ms=parseInt(this.value)||1;markDirty()"><span class="ms-lbl">ms</span>';
  }
  r+='</div></div>';
  return r;
}

function renderSteps(){
  var sc=document.getElementById('sc');
  if(sc)sc.textContent=steps.length+(steps.length===1?' step':' steps');
  var el=document.getElementById('step-list');
  if(el)el.innerHTML=steps.map(renderRow).join('');
}

function addStep(){
  steps.push({type:'key',key:'ENTER'});
  markDirty();renderSteps();
  var el=document.getElementById('step-list');
  if(el&&el.lastElementChild)el.lastElementChild.scrollIntoView({block:'nearest',behavior:'smooth'});
}
function rmStep(i){steps.splice(i,1);markDirty();renderSteps();}
function mv(i,d){var j=i+d;if(j<0||j>=steps.length)return;var t=steps[i];steps[i]=steps[j];steps[j]=t;markDirty();renderSteps();}
function chType(i,t){var s=steps[i];steps[i]=t==='key'?{type:'key',key:s.key||'ENTER'}:t==='combo'?{type:'combo',mods:['CTRL'],key:s.key||'C'}:t==='text'?{type:'text',text:s.text||''}:{type:'delay',ms:s.ms||500};markDirty();renderSteps();}
function togMod(i,mod){var s=steps[i];if(!s.mods)s.mods=[];var x=s.mods.indexOf(mod);if(x>=0)s.mods.splice(x,1);else s.mods.push(mod);markDirty();renderSteps();}

function applyWebColors(c){
  var r=document.documentElement.style;
  if(c.bg)r.setProperty('--clr-bg','#'+c.bg);
  if(c.surface)r.setProperty('--clr-surface','#'+c.surface);
  if(c.border)r.setProperty('--clr-border','#'+c.border);
  if(c.text)r.setProperty('--clr-text','#'+c.text);
  if(c.dim)r.setProperty('--clr-dim','#'+c.dim);
  if(c.accent)r.setProperty('--clr-accent','#'+c.accent);
}

async function saveFwColors(){
  mst('Saving...');
  var body=new URLSearchParams({
    bg:document.getElementById('fw-bg').value.replace('#',''),
    surface:document.getElementById('fw-surface').value.replace('#',''),
    accent:document.getElementById('fw-accent').value.replace('#',''),
    sel_bg:document.getElementById('fw-sel-bg').value.replace('#',''),
    text:document.getElementById('fw-text').value.replace('#',''),
    text_dim:document.getElementById('fw-text-dim').value.replace('#','')
  });
  try{
    var r=await fetch('/api/colors',{method:'POST',body:body});
    var j=await r.json();
    if(j.ok)mst('Display colors saved!','ok');
    else mst('Error: '+(j.err||'?'),'err');
  }catch(e){mst('Connection error','err');}
}

async function saveWebColors(){
  mst('Saving...');
  var vals={
    bg:document.getElementById('web-bg').value.replace('#',''),
    surface:document.getElementById('web-surface').value.replace('#',''),
    border:document.getElementById('web-border').value.replace('#',''),
    text:document.getElementById('web-text').value.replace('#',''),
    dim:document.getElementById('web-dim').value.replace('#',''),
    accent:document.getElementById('web-accent').value.replace('#','')
  };
  try{
    var r=await fetch('/api/webcolors',{method:'POST',body:new URLSearchParams(vals)});
    var j=await r.json();
    if(j.ok){applyWebColors(vals);mst('Web colors saved!','ok');}
    else mst('Error: '+(j.err||'?'),'err');
  }catch(e){mst('Connection error','err');}
}

async function load(){
  try{
    var r=await fetch('/api/macros');macros=await r.json();setOnline(true);
    document.getElementById('mc').textContent=macros.length;renderList();
    var ip=r.headers.get('X-Device-IP');if(ip)document.getElementById('ip-badge').textContent=ip;
  }catch(e){setOnline(false);st('Connection error','#cc4444');}
  fetch('/api/webcolors').then(function(r){return r.json();}).then(applyWebColors).catch(function(){});
}

function renderList(){
  document.getElementById('ml').innerHTML=macros.map(function(m,i){
    return'<div class="mi'+(i===cur?' active':'')+'" onclick="selMacro('+i+')" title="'+esc(m.name)+'">'
      +'<span class="mi-num">'+(i+1)+'</span>'
      +'<span class="mi-name">'+esc(m.name||'(no name)')+'</span>'
      +'<span class="ud'+(i===cur&&dirty?' show':'')+'">&#9679;</span></div>';
  }).join('');
}

function markDirty(){if(!dirty){dirty=true;renderList();}}

function edHTML(m,isNew){
  return'<div class="fg"><label>Name</label>'
    +'<input type="text" id="m-name" maxlength="32" value="'+(isNew?'':esc(m.name))+'" placeholder="'+(isNew?'Macro name':'')+'" oninput="markDirty()" autocomplete="off"></div>'
    +'<div class="fg"><div class="fh"><label>Steps</label><span class="sc" id="sc"></span></div>'
    +'<div class="step-list" id="step-list"></div>'
    +'<button class="add-btn" onclick="addStep()">+ Add step</button></div>'
    +'<div class="toolbar">'
    +(isNew?'<button class="btn-save" onclick="save()">+ Create</button>'
           :'<button class="btn-save" onclick="save()">&#10003; Save</button><button class="btn-del" onclick="del()">&#10007; Delete</button>')
    +'<span class="kh">Strg+S</span></div><div class="status" id="st"></div>';
}

function selMacro(i){
  if(dirty&&cur>=0&&!confirm('Discard changes?'))return;
  dirty=false;cur=i;renderList();closeSB();
  steps=parseSteps(macros[i].steps);
  document.getElementById('editor').innerHTML=edHTML(macros[i],false);
  renderSteps();
}

function newMacro(){
  if(dirty&&cur>=0&&!confirm('Discard changes?'))return;
  dirty=false;cur=-1;steps=[];renderList();closeSB();
  document.getElementById('editor').innerHTML=edHTML(null,true);
  renderSteps();markDirty();
}

async function save(){
  var name=(document.getElementById('m-name')||{}).value||'';
  name=name.trim();if(!name){st('Name cannot be empty.','#cc4444');return;}
  var body=new URLSearchParams({id:cur,name:name,steps:serializeSteps(steps)});
  try{
    var r=await fetch('/api/save',{method:'POST',body:body});var j=await r.json();
    if(j.ok){dirty=false;st('Saved!','#aaaaaa');var pv=cur;await load();cur=(pv===-1)?macros.length-1:pv;renderList();}
    else st('Error: '+(j.err||'?'),'#cc4444');
  }catch(e){st('Network error','#cc4444');}
}

async function del(){
  if(cur<0)return;
  if(!confirm('Delete macro "'+macros[cur].name+'"?'))return;
  try{
    var r=await fetch('/api/delete',{method:'POST',body:new URLSearchParams({id:cur})});var j=await r.json();
    if(j.ok){dirty=false;cur=-1;steps=[];await load();document.getElementById('editor').innerHTML='<div class="ph"><div class="ph-i">&#10006;</div><div class="ph-t">Deleted</div></div>';}
    else st('Error: '+(j.err||'?'),'#cc4444');
  }catch(e){st('Network error','#cc4444');}
}

function st(msg,color){
  var el=document.getElementById('st');if(!el)return;
  clearTimeout(stTimer);el.style.opacity='1';el.style.color=color||'#aaaaaa';el.textContent=msg;
  if(color==='#aaaaaa')stTimer=setTimeout(function(){el.style.opacity='0';},2200);
}

async function saveEnergy(){
  var en=document.getElementById('cfg-es-en').checked?'1':'0';
  var to=parseInt(document.getElementById('cfg-es-to').value)||30;
  var db=parseInt(document.getElementById('cfg-es-db').value)||10;
  var ab=parseInt(document.getElementById('cfg-es-ab').value)||128;
  var gm=document.getElementById('cfg-ss-gif').checked?'1':'0';
  mst('Saving...');
  try{
    var r=await fetch('/api/energy',{method:'POST',body:new URLSearchParams({enabled:en,timeout_s:to,dim_br:db,active_br:ab,gif_mode:gm})});
    var j=await r.json();
    if(j.ok)mst('Energy settings saved!','ok');
    else mst('Error: '+(j.err||'?'),'err');
  }catch(e){mst('Connection error','err');}
}

async function uploadGif(){
  var f=document.getElementById('ss-gif-file').files[0];
  if(!f){mst('Keine Datei ausgewählt.');return;}
  mst('Uploading...');
  var fd=new FormData();fd.append('file',f);
  try{
    var r=await fetch('/api/screensaver/upload',{method:'POST',body:fd});
    var j=await r.json();
    if(j.ok){mst('GIF hochgeladen!');var ssst=document.getElementById('ss-gif-st');if(ssst){ssst.textContent='GIF: hochgeladen \u2713';ssst.style.color='#aaaaaa';}}
    else mst('Fehler: '+(j.err||'?'));
  }catch(e){mst('Upload fehlgeschlagen.');}
}

async function deleteGif(){
  if(!confirm('Screensaver GIF löschen?'))return;
  mst('Löschen...');
  try{
    var r=await fetch('/api/screensaver/delete',{method:'POST'});
    var j=await r.json();
    if(j.ok){mst('GIF gelöscht.');var ssst=document.getElementById('ss-gif-st');if(ssst){ssst.textContent='Kein GIF hochgeladen';ssst.style.color='#666666';}}
    else mst('Fehler: '+(j.err||'?'));
  }catch(e){mst('Fehler.');}
}

document.addEventListener('keydown',function(e){if((e.ctrlKey||e.metaKey)&&e.key==='s'){e.preventDefault();save();}});
load();
</script>
</body>
</html>)rawliteral";

// ── JSON helper functions ─────────────────────────────────────────────────────

static String json_str(const String& s) {
    String out = "\"";
    for (int i = 0; i < (int)s.length(); i++) {
        char c = s[i];
        if (c == '"')       out += "\\\"";
        else if (c == '\\') out += "\\\\";
        else if (c == '\n') out += "\\n";
        else if (c == '\r') out += "";
        else                out += c;
    }
    out += "\"";
    return out;
}

// ── HTTP handlers ─────────────────────────────────────────────────────────────

static void handle_root() {
    server.send(200, "text/html; charset=utf-8", HTML_PAGE);
}

static void handle_api_macros() {
    String json = "[";
    int cnt = macro_store_count();
    for (int i = 0; i < cnt; i++) {
        const MacroInfo* m = macro_store_get(i);
        if (!m) continue;
        if (i > 0) json += ",";
        json += "{\"id\":" + String(i) +
                ",\"name\":" + json_str(String(m->name)) +
                ",\"steps\":" + json_str(macro_store_get_steps_raw(i)) +
                "}";
    }
    json += "]";
    server.sendHeader("X-Device-IP", web_server_ip());
    server.send(200, "application/json", json);
}

static void handle_api_save() {
    if (!server.hasArg("name") || !server.hasArg("steps") || !server.hasArg("id")) {
        server.send(400, "application/json", "{\"ok\":false,\"err\":\"Missing parameters\"}");
        return;
    }
    int  id    = server.arg("id").toInt();
    String name  = server.arg("name");
    String steps = server.arg("steps");

    name.trim();
    if (name.length() == 0) {
        server.send(400, "application/json", "{\"ok\":false,\"err\":\"Name empty\"}");
        return;
    }

    int result = macro_store_save(id, name, steps);
    if (result < 0) {
        server.send(500, "application/json", "{\"ok\":false,\"err\":\"Save failed\"}");
        return;
    }

    ui_reload();
    server.send(200, "application/json", "{\"ok\":true,\"id\":" + String(result) + "}");
}

static void handle_api_delete() {
    if (!server.hasArg("id")) {
        server.send(400, "application/json", "{\"ok\":false,\"err\":\"Missing ID\"}");
        return;
    }
    int id = server.arg("id").toInt();
    if (!macro_store_delete(id)) {
        server.send(500, "application/json", "{\"ok\":false,\"err\":\"Delete failed\"}");
        return;
    }

    ui_reload();
    server.send(200, "application/json", "{\"ok\":true}");
}

static void handle_api_status() {
    String json = "{";
    json += "\"fs_ok\":"    + String(macro_store_fs_ok() ? "true" : "false");
    json += ",\"count\":"   + String(macro_store_count());
    json += ",\"files\":"   + json_str(macro_store_list_files());
    json += "}";
    server.send(200, "application/json", json);
}

static void handle_api_settings_get() {
    Preferences prefs;
    prefs.begin("wifi", true);
    String ssid = prefs.getString("ssid", WIFI_AP_SSID);
    prefs.end();
    server.send(200, "application/json", "{\"ssid\":" + json_str(ssid) + "}");
}

static void handle_api_settings_post() {
    if (!server.hasArg("ssid")) {
        server.send(400, "application/json", "{\"ok\":false,\"err\":\"Missing parameters\"}");
        return;
    }
    String ssid = server.arg("ssid");
    String pass = server.hasArg("pass") ? server.arg("pass") : "";
    ssid.trim();

    if (ssid.length() == 0) {
        server.send(400, "application/json", "{\"ok\":false,\"err\":\"SSID empty\"}");
        return;
    }
    if (pass.length() > 0 && pass.length() < 8) {
        server.send(400, "application/json", "{\"ok\":false,\"err\":\"Password too short (min. 8 characters)\"}");
        return;
    }

    Preferences prefs;
    prefs.begin("wifi", false);
    prefs.putString("ssid", ssid);
    if (pass.length() >= 8) {
        prefs.putString("pass", pass);
    }
    prefs.end();

    server.send(200, "application/json", "{\"ok\":true}");
    delay(500);
    ESP.restart();
}

static void handle_api_restart() {
    server.send(200, "application/json", "{\"ok\":true}");
    delay(500);
    ESP.restart();
}

static void handle_api_restart_bootloader() {
    server.send(200, "application/json", "{\"ok\":true}");
    delay(500);
    // Sets the FORCE_DOWNLOAD_BOOT flag so the ROM bootloader is active on next start
    REG_WRITE(RTC_CNTL_OPTION1_REG, RTC_CNTL_FORCE_DOWNLOAD_BOOT);
    esp_restart();
}

static void handle_api_energy_get() {
    Preferences prefs;
    prefs.begin("energy", true);
    bool     enabled   = prefs.getBool("enabled",   true);
    uint32_t timeout_s = prefs.getUInt("timeout_s", ENERGY_SAVE_TIMEOUT_DEFAULT);
    uint8_t  dim_br    = prefs.getUInt("dim_br",    ENERGY_SAVE_DIM_BRIGHTNESS);
    uint8_t  active_br = prefs.getUInt("active_br", ENERGY_SAVE_ACTIVE_BRIGHTNESS);
    prefs.end();
    String json = "{\"enabled\":"    + String(enabled ? "true" : "false") +
                  ",\"timeout_s\":"  + String(timeout_s) +
                  ",\"dim_br\":"     + String(dim_br) +
                  ",\"active_br\":"  + String(active_br) +
                  ",\"gif_mode\":"   + String(energy_save_screensaver_gif_mode() ? "true" : "false") + "}";
    server.send(200, "application/json", json);
}

static void handle_api_energy_post() {
    Preferences prefs;
    prefs.begin("energy", false);
    if (server.hasArg("enabled"))
        prefs.putBool("enabled", server.arg("enabled") == "1");
    if (server.hasArg("timeout_s")) {
        int t = server.arg("timeout_s").toInt();
        if (t < 5)   t = 5;
        if (t > 3600) t = 3600;
        prefs.putUInt("timeout_s", (uint32_t)t);
    }
    if (server.hasArg("dim_br"))
        prefs.putUInt("dim_br", (uint32_t)constrain(server.arg("dim_br").toInt(), 0, 255));
    if (server.hasArg("active_br"))
        prefs.putUInt("active_br", (uint32_t)constrain(server.arg("active_br").toInt(), 10, 255));
    if (server.hasArg("gif_mode"))
        prefs.putBool("ss_gif", server.arg("gif_mode") == "1");
    prefs.end();

    energy_save_init();   // Apply new settings immediately
    server.send(200, "application/json", "{\"ok\":true}");
}

static void handle_api_colors_get() {
    Preferences prefs;
    prefs.begin("colors", true);
    uint32_t bg       = prefs.getUInt("bg",       0x000000);
    uint32_t surface  = prefs.getUInt("surface",  0x000000);
    uint32_t accent   = prefs.getUInt("accent",   0x808080);
    uint32_t sel_bg   = prefs.getUInt("sel_bg",   0x1A1A1A);
    uint32_t text     = prefs.getUInt("text",     0xFFFFFF);
    uint32_t text_dim = prefs.getUInt("text_dim", 0x888888);
    prefs.end();
    char buf[160];
    snprintf(buf, sizeof(buf),
        "{\"bg\":\"%06X\",\"surface\":\"%06X\",\"accent\":\"%06X\","
        "\"sel_bg\":\"%06X\",\"text\":\"%06X\",\"text_dim\":\"%06X\"}",
        (unsigned)bg, (unsigned)surface, (unsigned)accent,
        (unsigned)sel_bg, (unsigned)text, (unsigned)text_dim);
    server.send(200, "application/json", buf);
}

static void handle_api_colors_post() {
    auto parseHex = [](const String& s) -> uint32_t {
        String v = s; v.trim();
        if (v.startsWith("#")) v = v.substring(1);
        return (uint32_t)strtoul(v.c_str(), nullptr, 16);
    };
    Preferences prefs;
    prefs.begin("colors", false);
    if (server.hasArg("bg"))       prefs.putUInt("bg",       parseHex(server.arg("bg")));
    if (server.hasArg("surface"))  prefs.putUInt("surface",  parseHex(server.arg("surface")));
    if (server.hasArg("accent"))   prefs.putUInt("accent",   parseHex(server.arg("accent")));
    if (server.hasArg("sel_bg"))   prefs.putUInt("sel_bg",   parseHex(server.arg("sel_bg")));
    if (server.hasArg("text"))     prefs.putUInt("text",     parseHex(server.arg("text")));
    if (server.hasArg("text_dim")) prefs.putUInt("text_dim", parseHex(server.arg("text_dim")));
    prefs.end();
    ui_apply_colors();
    server.send(200, "application/json", "{\"ok\":true}");
}

static void handle_api_webcolors_get() {
    Preferences prefs;
    prefs.begin("webclr", true);
    uint32_t bg      = prefs.getUInt("bg",      0x0A0A0A);
    uint32_t surface = prefs.getUInt("surface",  0x111111);
    uint32_t border  = prefs.getUInt("border",   0x222222);
    uint32_t text    = prefs.getUInt("text",     0xE0E0E0);
    uint32_t dim     = prefs.getUInt("dim",      0x888888);
    uint32_t accent  = prefs.getUInt("accent",   0x888888);
    prefs.end();
    char buf[140];
    snprintf(buf, sizeof(buf),
        "{\"bg\":\"%06X\",\"surface\":\"%06X\",\"border\":\"%06X\","
        "\"text\":\"%06X\",\"dim\":\"%06X\",\"accent\":\"%06X\"}",
        (unsigned)bg, (unsigned)surface, (unsigned)border,
        (unsigned)text, (unsigned)dim, (unsigned)accent);
    server.send(200, "application/json", buf);
}

static void handle_api_webcolors_post() {
    auto parseHex = [](const String& s) -> uint32_t {
        String v = s; v.trim();
        if (v.startsWith("#")) v = v.substring(1);
        return (uint32_t)strtoul(v.c_str(), nullptr, 16);
    };
    Preferences prefs;
    prefs.begin("webclr", false);
    if (server.hasArg("bg"))      prefs.putUInt("bg",      parseHex(server.arg("bg")));
    if (server.hasArg("surface")) prefs.putUInt("surface", parseHex(server.arg("surface")));
    if (server.hasArg("border"))  prefs.putUInt("border",  parseHex(server.arg("border")));
    if (server.hasArg("text"))    prefs.putUInt("text",    parseHex(server.arg("text")));
    if (server.hasArg("dim"))     prefs.putUInt("dim",     parseHex(server.arg("dim")));
    if (server.hasArg("accent"))  prefs.putUInt("accent",  parseHex(server.arg("accent")));
    prefs.end();
    server.send(200, "application/json", "{\"ok\":true}");
}

static void handle_api_screensaver_get() {
    bool has_gif  = LittleFS.exists("/screensaver.gif");
    bool gif_mode = energy_save_screensaver_gif_mode();
    server.send(200, "application/json",
        "{\"has_gif\":"  + String(has_gif  ? "true" : "false") +
        ",\"gif_mode\":" + String(gif_mode ? "true" : "false") + "}");
}

static File s_upload_file;

static void handle_screensaver_upload_done() {
    bool ok = LittleFS.exists("/screensaver.gif");
    server.send(200, "application/json",
        ok ? "{\"ok\":true}" : "{\"ok\":false,\"err\":\"Upload failed\"}");
}

static void handle_screensaver_upload() {
    HTTPUpload& upload = server.upload();
    if (upload.status == UPLOAD_FILE_START) {
        s_upload_file = LittleFS.open("/screensaver.gif", "w");
    } else if (upload.status == UPLOAD_FILE_WRITE) {
        if (s_upload_file) s_upload_file.write(upload.buf, upload.currentSize);
    } else if (upload.status == UPLOAD_FILE_END) {
        if (s_upload_file) s_upload_file.close();
    }
}

static void handle_api_screensaver_delete() {
    LittleFS.remove("/screensaver.gif");
    energy_save_notify_gif_changed();
    server.send(200, "application/json", "{\"ok\":true}");
}

static void handle_not_found() {
    server.send(404, "text/plain", "Not found");
}

// ── Public API ────────────────────────────────────────────────────────────────

void web_server_init() {
    // Read SSID and password from NVS (fallback: config.h constants)
    Preferences prefs;
    prefs.begin("wifi", true);
    String ssid = prefs.getString("ssid", WIFI_AP_SSID);
    String pass = prefs.getString("pass", WIFI_AP_PASS);
    prefs.end();

    WiFi.mode(WIFI_AP);
    WiFi.softAP(ssid.c_str(), pass.c_str());

    server.on("/",                       HTTP_GET,  handle_root);
    server.on("/api/macros",             HTTP_GET,  handle_api_macros);
    server.on("/api/status",             HTTP_GET,  handle_api_status);
    server.on("/api/settings",           HTTP_GET,  handle_api_settings_get);
    server.on("/api/save",               HTTP_POST, handle_api_save);
    server.on("/api/delete",             HTTP_POST, handle_api_delete);
    server.on("/api/settings",           HTTP_POST, handle_api_settings_post);
    server.on("/api/restart",            HTTP_POST, handle_api_restart);
    server.on("/api/restart-bootloader", HTTP_POST, handle_api_restart_bootloader);
    server.on("/api/energy",             HTTP_GET,  handle_api_energy_get);
    server.on("/api/energy",             HTTP_POST, handle_api_energy_post);
    server.on("/api/colors",             HTTP_GET,  handle_api_colors_get);
    server.on("/api/colors",             HTTP_POST, handle_api_colors_post);
    server.on("/api/webcolors",          HTTP_GET,  handle_api_webcolors_get);
    server.on("/api/webcolors",          HTTP_POST, handle_api_webcolors_post);
    server.on("/api/screensaver",        HTTP_GET,  handle_api_screensaver_get);
    server.on("/api/screensaver/upload", HTTP_POST, handle_screensaver_upload_done, handle_screensaver_upload);
    server.on("/api/screensaver/delete", HTTP_POST, handle_api_screensaver_delete);
    server.onNotFound(handle_not_found);

    server.begin();
}

void web_server_handle() {
    server.handleClient();
}

String web_server_ip() {
    return WiFi.softAPIP().toString();
}
