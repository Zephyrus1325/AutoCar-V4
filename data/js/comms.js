
let ws;
let tInicioPing;
let pingTimer;
let reconnectTimer;
let urlWS = 'ws://' + "autocar.local" + '/ws';

function conectarWS() {
    if (ws && ws.readyState !== WebSocket.CLOSED) return;

    const socket = new WebSocket(urlWS);
    ws = socket;
    socket.binaryType = "arraybuffer";

    socket.onopen = () => {
        if (socket !== ws) return;

        clearTimeout(reconnectTimer);
        reconnectTimer = undefined;
        clearInterval(pingTimer);
        document.getElementById('net-status').innerText = "CONECTADO";
        document.getElementById('net-status').style.color = varColor('--primaria');
        adicionarLogConsole("Conexão WebSocket estabelecida!");

        desenharArco('gaugeEsquerdo', 0, 60, varColor('--primaria'));
        desenharArco('gaugeDireito', 0, 60, varColor('--primaria'));
        desenharArco('gaugeLidar', 0, 450, varColor('--azul'));

        pingTimer = setInterval(() => {
            if (socket !== ws || socket.readyState !== WebSocket.OPEN) return;
            tInicioPing = performance.now();
            socket.send(JSON.stringify({type:"ping"}));
        }, 2000);
    };

    socket.onclose = () => {
        if (socket !== ws) return;

        clearInterval(pingTimer);
        pingTimer = undefined;
        ws.close();
        ws = undefined;
        document.getElementById('net-status').innerText = "DESCONECTADO";
        document.getElementById('net-status').style.color = varColor('--perigo');
        console.log("DED");
        adicionarLogConsole("Conexão perdida. Tentando reconectar...");
        //if (!reconnectTimer) {
        //    reconnectTimer = setTimeout(() => {
        //        reconnectTimer = undefined;
        //        conectarWS();
        //    }, 2000);
        //}
        console.log("WebSocket connection closed.");
        console.log("Close Code:", event.code);       // Standardized integer code
        console.log("Reason:", event.reason);         // Optional text description provided by the server
        console.log("Was Clean:", event.wasClean);     // Boolean indicating if it was a clean shutdown

    };

    socket.onmessage = (event) => {
        if (typeof event.data === "string") {
            const json = JSON.parse(event.data);
            
            if (json.type === "pong") {
                document.getElementById('net-ping').innerText = Math.round(performance.now() - tInicioPing) + " ms";
                document.getElementById('net-rssi').innerText = json.rssi + " dBm";
                document.getElementById('battery_voltage').innerText = json.battery.toFixed(1) + " V";

                if(json.battery < 10.5){
                    document.getElementById('battery_voltage').classList.add("critical");
                    document.getElementById('battery_voltage').classList.remove("alert");
                } else if(json.battery < 11.2){
                    document.getElementById('battery_voltage').classList.add("alert");
                    document.getElementById('battery_voltage').classList.remove("critical");
                } else {
                    document.getElementById('battery_voltage').classList.remove("alert");
                    document.getElementById('battery_voltage').classList.remove("critical");
                }
            }
            else if (json.type === "telemetry") {
                // Intercepta e renderiza texto no console caso exista no JSON
                if (json.log) {
                    adicionarLogConsole(json.log);
                }

                if (json.motor != null) {
                    document.getElementById('motE-real').innerText = json.motor.left.speed.toFixed(1) + " cm/s";
                    document.getElementById('motE-set').innerText = json.motor.left.setpoint.toFixed(1) + " cm/s";
                    document.getElementById('motE-pwm').innerText = (json.motor.left.throttle * 100.0).toFixed(1) + " %";
                    
                    document.getElementById('motD-real').innerText = json.motor.right.speed.toFixed(1) + " cm/s";
                    document.getElementById('motD-set').innerText = json.motor.right.setpoint.toFixed(1) + " cm/s";
                    document.getElementById('motD-pwm').innerText = (json.motor.right.throttle * 100.0).toFixed(1) + " %";
                                                                                     
                    if (json.motor.left.speed < 0) {
                        desenharArco('gaugeEsquerdo', Math.abs(json.motor.left.speed), 60, varColor('--perigo'));
                    } else {
                        desenharArco('gaugeEsquerdo', Math.abs(json.motor.left.speed), 60, varColor('--primaria'));
                    }
                    
                    if (json.motor.right.speed < 0) {
                        desenharArco('gaugeDireito', Math.abs(json.motor.right.speed), 60, varColor('--perigo'));
                    } else {
                        desenharArco('gaugeDireito', Math.abs(json.motor.right.speed), 60, varColor('--primaria'));
                    }
                }
                
                if (json.lidar != null) {
                    document.getElementById('lidar-real').innerText = json.lidar.rpm;
                    if(json.lidar.rpm < 150){
                        document.getElementById('lidar-real').classList.add("blink_alert");
                    } else {
                        document.getElementById('lidar-real').classList.remove("blink_alert");
                    }
                    
                    document.getElementById('lidar-set').innerText = json.lidar.setpoint;
                    document.getElementById('lidar-pwm').innerText = (json.lidar.throttle * 100.0).toFixed(1) + " %";
                    desenharArco('gaugeLidar', json.lidar.rpm, 600, varColor('--azul'));
                }     
                
                if (json.navigation != null){
                    robot_x = json.navigation.position.x;
                    robot_y = json.navigation.position.y;
                    robot_heading = json.navigation.position.heading;
                }
            }
        } else if (event.data instanceof ArrayBuffer) {
            const buffer = event.data;
            const view = new DataView(buffer);

            let offset = 0;

            const data_type = view.getUint16(offset, true); offset += 2;
            
            if(data_type == 1){ // Lidar reading
                const length = view.getUint16(offset, true);
                offset += 2;

                const angles = [];
                for(let i = 0; i < length; i++){
                    angles[i] = view.getFloat32(offset, true);
                    offset += 4;
                }

                const distances = [];
                for(let i = 0; i < length; i++){
                    distances[i] = view.getFloat32(offset, true);
                    offset += 4;
                }

                // ==========================================
                // CÁLCULO GEOMÉTRICO DOS PONTOS (NOVO)
                // ==========================================

                for (let i = 0; i < length; i++) {
                    const distMm = distances[i] * 10; // Distância bruta vinda do Lidar (em mm)

                    // Ignora leituras inválidas (Ex: menores que 10cm ou maiores que 4 metros)
                    if (distMm <= 150 || distMm > 4000) continue; 
                
                    let anguloLocal = angles[i];
                    let anguloGlobal = anguloLocal + robot_heading;
                
                    // CONVERSÃO DE MM PARA PIXELS ANTES DE CALCULAR A POSIÇÃO GLOBAL
                    let distPixels = distMm * PIXELS_POR_MM;
                
                    // Agora o X e Y calculados estarão na escala correta do Canvas
                    let obstaculoX = robot_x + (distPixels * Math.cos(anguloGlobal));
                    let obstaculoY = robot_y + (distPixels * Math.sin(anguloGlobal));
                
                    pontosLidarGlobal.push({ x: obstaculoX, y: obstaculoY });
                }

                // Limita o histórico de pontos para não travar a renderização do navegador após horas rodando
                if (pontosLidarGlobal.length > 400) {
                    pontosLidarGlobal.splice(0, pontosLidarGlobal.length - 400);
                }

            } else if(data_type == 2){  // Chunk data
                
                
                const chunk_x = view.getInt16(offset, true); offset += 2;
                const chunk_y = view.getInt16(offset, true); offset += 2;
                const part = view.getUint16(offset, true); offset += 2;
                
                //console.log("chunk_x: " + chunk_x + " | chunk_y: " + chunk_y + " part: " + part);

                let base_x = chunk_x * 128 * 40;    // Corrigir isso eventualmente... -_-
                let base_y = chunk_y * 128 * 40;    //

                let len = Math.trunc(128 * 128 / 16);
                let start_id = part * len;

                for(let i = start_id; i < start_id + len; i++){
                    let x = base_x + Math.trunc(i % 128) * 40;
                    let y = base_y + Math.trunc(i / 128) * 40;
                    let value = view.getUint8(offset, true); offset += 1;
                    let key = x + "_" + y;
                    mapaGradeGlobal[key] = value;
                }
            }
        }
        desenharMapa();
    };

    ws.onerror = function(error) {
        console.error("WebSocket encountered an error: ", error);
    };
}

setInterval(() => {
    if(ws === undefined){
        console.log("Tried Connection");
        conectarWS();
    }   
}, 1000);

// O segredo está aqui: interceptar o evento antes da página descarregar
window.addEventListener('beforeunload', () => {
    if (ws && ws.readyState === WebSocket.OPEN) {
        ws.close(); // Avisa o ESP32 que está fechando voluntariamente
    }
});

window.addEventListener('load', conectarWS);

/* ========================================================================
   5. SISTEMA DE TROCA DE PÁGINAS
   ======================================================================== */
var active_page = "map_screen"
function mudarPagina(nomePagina) {
    document.querySelectorAll('.btn-nav').forEach(btn => btn.classList.remove('active'));
    if(event) event.target.classList.add('active');

    document.getElementById("map_screen").style.display = "none";
    document.getElementById("config_screen").style.display = "none";
    document.getElementById("pid_screen").style.display = "none";

    document.getElementById(nomePagina).style.display = "";

    console.log("Mudando para a página:", nomePagina);
}


/* ========================================================================
   6. CONTROLE TELEOPERADO VIA TECLADO (WASD) - TEMPORÁRIO
   ======================================================================== */
// Define as velocidades base para o seu motor GA25 (em cm/s ou RPM, dependendo do seu PID)
const VELOCIDADE_LINEAR = 30.0;  // Velocidade para frente/trás
const VELOCIDADE_ANGULAR = 10.0; // Velocidade de giro

// Estrutura para rastrear quais teclas estão pressionadas
let teclasPressionadas = { w: false, a: false, s: false, d: false };

window.addEventListener('keydown', (e) => {
    let tecla = e.key.toLowerCase();
    if (teclasPressionadas.hasOwnProperty(tecla) && !teclasPressionadas[tecla]) {
        teclasPressionadas[tecla] = true;
        enviarComandoMotor();
    }
});

window.addEventListener('keyup', (e) => {
    let tecla = e.key.toLowerCase();
    if (teclasPressionadas.hasOwnProperty(tecla)) {
        teclasPressionadas[tecla] = false;
        enviarComandoMotor();
    }
});

function enviarComandoMotor() {
    let velEsq = 0;
    let velDir = 0;

    if (teclasPressionadas.w) { // Frente
        velEsq += VELOCIDADE_LINEAR;
        velDir += VELOCIDADE_LINEAR;
    }
    if (teclasPressionadas.s) { // Trás
        velEsq += -VELOCIDADE_LINEAR;
        velDir += -VELOCIDADE_LINEAR;
    }
    if (teclasPressionadas.a) { // Girar para Esquerda no próprio eixo
        velEsq += -VELOCIDADE_ANGULAR;
        velDir += VELOCIDADE_ANGULAR;   // VELOCIDADES ANGULAS SENDO CALCULADAS ERRADO - ALTERAR PARA FORMULA CERTA EVENUTALMENTE.
    }
    if (teclasPressionadas.d) { // Girar para Direita no próprio eixo
        velEsq += VELOCIDADE_ANGULAR;
        velDir += -VELOCIDADE_ANGULAR;
    }

    // Só envia se o WebSocket estiver conectado
    if (ws && ws.readyState === WebSocket.OPEN) {
        ws.send(JSON.stringify({
            type: "cmd_vel",
            left: velEsq,
            right: velDir
        }));
    }
}

function enviarDestino(dest_x, dest_y){
    // Só envia se o WebSocket estiver conectado
    if (ws && ws.readyState === WebSocket.OPEN) {
        ws.send(JSON.stringify({
            type: "cmd_destination",
            x: dest_x,
            y: dest_y
        }));
        adicionarLogConsole("Send: " + dest_x + " | " + dest_y);
    }
}