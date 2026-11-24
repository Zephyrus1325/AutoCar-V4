// ESCALA: 1 mm real = 0.1 pixels na tela (10mm = 1px)
const PIXELS_POR_MM = 0.1; 
const TAMANHO_GRID_MM = 40; // Quadrados de 40x40 mm

let pontosLidarGlobal = []; // Armazena os pontos históricos do Lidar no mapa global [{x, y}, {x, y}, ...]
let mapaGradeGlobal = {};   // Banco de dados dos quadrados do mapa. Chave: "X_Y", Valor: byte de 0 a 255

function obterCorOcupacao(valor) {
    // 0 = Livre (Verde), 127 = Desconhecido (Cinza Escuro), 255 = Parede (Vermelho Vivo)
    if (valor === 127) {
        return "rgba(40, 40, 45, 0.3)"; // Transparente/Cinza para o desconhecido
    } else if (valor < 127) {
        // Interpola de Verde (0) até o Cinza (127)
        let pct = valor / 127;
        let r = Math.round(40 + (pct * 0));
        let g = Math.round(180 - (pct * 140));
        let b = Math.round(40 + (pct * 5));
        return `rgba(${r}, ${g}, ${b}, 0.8)`;
    } else {
        // Interpola de Cinza (127) até Vermelho/Preto (255)
        let pct = (valor - 127) / 128;
        let r = Math.round(40 + (pct * 215));
        let g = Math.round(40 - (pct * 20));
        let b = Math.round(45 - (pct * 25));
        return `rgba(${r}, ${g}, ${b}, 0.9)`;
    }
}
/* ========================================================================
   CANVAS PRINCIPAL E CANVAS DE CACHE (OFFSCREEN)
   ======================================================================== */
const mapCanvas = document.getElementById('mapCanvas');
const mctx = mapCanvas.getContext('2d');

let camera = { x: mapCanvas.width / 2, y: mapCanvas.height / 2, zoom: 1.0 };
let mapaChunks = [ {x: 0, y: 0}, {x: 100, y: -80} ];

// Criamos um canvas invisível para servir de buffer estático do grid
const cacheCanvas = document.createElement('canvas');
const cctx = cacheCanvas.getContext('2d');

// Controle para saber quando o grid realmente mudou e precisa ser redesenhado no cache
let gridNecessitaRedesenho = true;

function redimensionarMapa() {
    mapCanvas.width = mapCanvas.parentElement.clientWidth;
    mapCanvas.height = mapCanvas.parentElement.clientHeight;
    
    // O tamanho do cache do mapa global (ex: 4000x4000 pixels para cobrir a área do robô)
    cacheCanvas.width = 4000;
    cacheCanvas.height = 4000;
    
    gridNecessitaRedesenho = true;
}
redimensionarMapa();
window.addEventListener('resize', () => { redimensionarMapa(); desenharMapa(); });

// Armazene os dados convertendo a chave em objeto direto no WebSocket para evitar split()
// Quando receber o chunk no WebSocket, mude para alimentar o objeto e setar a flag:
// mapaGradeGlobal[key] = { x: mmX, y: mmY, v: value };
// gridNecessitaRedesenho = true;

/* ========================================================================
   FUNÇÃO AUXILIAR: ATUALIZA APENAS O GRID ESTÁTICO (CHUNKS)
   ======================================================================== */
function renderizarGridNoCache() {
    cctx.clearRect(0, 0, cacheCanvas.width, cacheCanvas.height);
    
    // Centraliza a origem (0,0) do robô no meio do canvas de cache
    cctx.save();
    cctx.translate(cacheCanvas.width / 2, cacheCanvas.height / 2);
    
    const espacamentoPixels = TAMANHO_GRID_MM * PIXELS_POR_MM;

    // 1. DESENHA OS CHUNKS
    for (let chave in mapaGradeGlobal) {
        let dados = mapaGradeGlobal[chave];
        // Se mudou o parser no WebSocket para objeto direto:
        // let pixelX = dados.x * PIXELS_POR_MM;
        // let pixelY = dados.y * PIXELS_POR_MM;
        // let valor = dados.v;
        
        // Se manteve string por enquanto, o split roda apenas aqui (fora do loop principal de render)
        let [mmX, mmY] = chave.split('_').map(Number);
        let valor = mapaGradeGlobal[chave];
        let pixelX = mmX * PIXELS_POR_MM;
        let pixelY = mmY * PIXELS_POR_MM;

        cctx.fillStyle = obterCorOcupacao(valor);
        cctx.fillRect(pixelX, -pixelY, espacamentoPixels, espacamentoPixels);
    }

    // 2. DESENHA AS LINHAS DE SUPORTE (Rodando uma única vez aqui)
    cctx.strokeStyle = 'rgba(255, 255, 255, 0.03)';
    cctx.lineWidth = 0.5;
    const limiteMapa = 2000;
    cctx.beginPath();
    for (let i = -limiteMapa; i <= limiteMapa; i += espacamentoPixels) {
        cctx.moveTo(i, -limiteMapa); cctx.lineTo(i, limiteMapa);
        cctx.moveTo(-limiteMapa, i); cctx.lineTo(limiteMapa, i);
    }
    cctx.stroke();

    cctx.restore();
    gridNecessitaRedesenho = false; // Cache atualizado!
}


/* ========================================================================
   RENDERIZADOR DO MAPA PRINCIPAL (CHAMADO A CADA FRAME - ULTRA RÁPIDO)
   ======================================================================== */
function desenharMapa() {
    // Se houve novos dados de Chunk, atualiza a imagem estática de fundo primeiro
    if (gridNecessitaRedesenho) {
        renderizarGridNoCache();
    }

    // Limpa o canvas visível da tela
    mctx.setTransform(1, 0, 0, 1, 0, 0);
    mctx.clearRect(0, 0, mapCanvas.width, mapCanvas.height);
    
    // Aplica Pan e Zoom da Câmera
    mctx.translate(camera.x, camera.y);
    mctx.scale(camera.zoom, camera.zoom);

    // ========================================================================
    // PASSO 1: DESENHA O MAPA INTEIRO VIA HARDWARE (Apenas joga o cache na tela)
    // ========================================================================
    // drawImage usa aceleração gráfica da GPU. É instantâneo, mesmo com zoom.
    mctx.drawImage(cacheCanvas, -cacheCanvas.width / 2, -cacheCanvas.height / 2);

    // ========================================================================
    // PASSO 2: DESENHA OS ELEMENTOS DINÂMICOS (Nuvem do Lidar)
    // ========================================================================
    mctx.fillStyle = '#ff6b6b';
    const raioPonto = 2 / camera.zoom;
    
    // Otimização: Agrupa os desenhos em um único path para enviar de uma vez para a GPU
    mctx.beginPath();
    for (let i = 0; i < pontosLidarGlobal.length; i++) {
        let p = pontosLidarGlobal[i]; 
        mctx.moveTo(p.x, -p.y);
        mctx.arc(p.x, -p.y, raioPonto, 0, 2 * Math.PI);
    }
    mctx.fill();

    // ========================================================================
    // PASSO 3: DESENHA O ROBÔ (Posição Dinâmica)
    // ========================================================================
    mctx.save();
    mctx.translate(robot_x, -robot_y);
    mctx.rotate(-robot_heading);

    mctx.fillStyle = varColor('--primaria');
    mctx.beginPath();
    mctx.arc(0, 0, 12, 0, 2 * Math.PI);
    mctx.fill();

    mctx.strokeStyle = '#fff';
    mctx.lineWidth = 2 / camera.zoom;
    mctx.beginPath(); mctx.moveTo(0, 0); mctx.lineTo(15, 0); mctx.stroke();

    mctx.restore();
}

function varColor(name) { return getComputedStyle(document.documentElement).getPropertyValue(name).trim(); }

let arrastando = false, mouseX, mouseY;
mapCanvas.addEventListener('mousedown', (e) => { arrastando = true; mouseX = e.clientX - camera.x; mouseY = e.clientY - camera.y; });
mapCanvas.addEventListener('mousemove', (e) => { if(!arrastando) return; camera.x = e.clientX - mouseX; camera.y = e.clientY - mouseY; desenharMapa(); });
window.addEventListener('mouseup', () => arrastando = false);
mapCanvas.addEventListener('wheel', (e) => {
    e.preventDefault();
    const mX = e.clientX, mY = e.clientY;
    const mundoX = (mX - camera.x) / camera.zoom, mundoY = (mY - camera.y) / camera.zoom;
    camera.zoom = Math.max(0.3, Math.min(4.0, camera.zoom * (e.deltaY < 0 ? 1.1 : 0.9)));
    camera.x = mX - mundoX * camera.zoom; camera.y = mY - mundoY * camera.zoom;
    desenharMapa();
}, { passive: false });
desenharMapa();

mapCanvas.addEventListener('dblclick', (e) => {
    // 1. Pega as coordenadas do clique relativas à borda do Canvas
    const rect = mapCanvas.getBoundingClientRect();
    const mouseTelaX = e.clientX - rect.left;
    const mouseTelaY = e.clientY - rect.top;

    // 2. DESFAZ O PAN E ZOOM DA CÂMERA (Converte Screen -> World em Pixels)
    const pixelMundoX = (mouseTelaX - camera.x) / camera.zoom;
    const pixelMundoY = (mouseTelaY - camera.y) / camera.zoom;

    // 3. CONVERTE DE PIXELS PARA MM (E inverte o eixo Y para bater com sua renderização)
    // No seu código você desenha: mctx.drawImage(cacheCanvas, -cacheCanvas.width / 2, ...)
    // O centro do cacheCanvas está alinhado com a origem do mundo.
    const mmX = pixelMundoX / PIXELS_POR_MM;
    const mmY = -pixelMundoY / PIXELS_POR_MM; // Note o sinal de menos para reverter o -Y do desenho

    // 4. DESCOBRE A CHAVE DO GRID (Baseado no TAMANHO_GRID_MM, ex: 40mm)
    // Usamos Math.floor para mapear corretamente inclusive em coordenadas negativas
    const gridX = Math.floor(mmX / TAMANHO_GRID_MM) * TAMANHO_GRID_MM;
    const gridY = Math.floor(mmY / TAMANHO_GRID_MM) * TAMANHO_GRID_MM;
    const chaveGrid = `${gridX}_${gridY}`;

    // 5. VERIFICA O STATUS DAQUELE QUADRADO NO SEU MAPA
    let valorOcupacao = mapaGradeGlobal[chaveGrid];
    if (valorOcupacao === undefined) valorOcupacao = 127; // Desconhecido se não existir no objeto

    // EXEMPLO DE USO: Printar no seu console customizado
    //adicionarLogConsole(`Clique em MM: X=${(mmX/10).toFixed(1)}, Y=${(mmY/10).toFixed(1)}`);
    //adicionarLogConsole(`Grid Chave: "${chaveGrid}" | Ocupação: ${valorOcupacao}`);
    enviarDestino(mmX/10, mmY/10)
    // Se quiser disparar um evento para enviar uma rota/waypoint para o robô:
    // enviarWaypointAoRobo(mmX, mmY);
});

/* ========================================================================
   2. RENDERIZADOR DE ARCOS (GAUGES DINÂMICOS EM CANVAS)
   ======================================================================== */
function desenharArco(canvasId, valor, valorMax, corHex) {
    const canvas = document.getElementById(canvasId);
    const ctx = canvas.getContext('2d');
    const cx = canvas.width / 2;
    const cy = canvas.height - 10;
    const r = (canvas.width / 2) - 15;

    ctx.clearRect(0, 0, canvas.width, canvas.height);
    
    ctx.beginPath(); ctx.arc(cx, cy, r, Math.PI, 2 * Math.PI, false);
    ctx.lineWidth = 10; ctx.strokeStyle = '#29292e'; ctx.stroke();

    let porcentagem = Math.min(1, Math.max(0, valor / valorMax));
    if (porcentagem > 0) {
        ctx.beginPath();
        ctx.arc(cx, cy, r, Math.PI, Math.PI + (porcentagem * Math.PI), false);
        ctx.lineWidth = 10; ctx.strokeStyle = corHex; ctx.stroke();
    }
}

setInterval(() =>{
    gridNecessitaRedesenho = true;
}, 500);

/* ========================================================================
   3. GERENCIADOR DO TERMINAL / CONSOLE (NOVO)
   ======================================================================== */
const consoleDiv = document.getElementById('robotConsole');

function adicionarLogConsole(mensagem) {
    const agora = new Date();
    const timestamp = `[${agora.toTimeString().split(' ')[0]}]`;
    
    // Cria o elemento de linha do log
    const linha = document.createElement('div');
    linha.className = 'console-line';
    linha.innerHTML = `<span class="console-time">${timestamp}</span>${mensagem}`;
    
    consoleDiv.appendChild(linha);

    // Limita o console a 60 linhas para preservar performance do navegador
    if (consoleDiv.children.length > 60) {
        consoleDiv.removeChild(consoleDiv.firstChild);
    }

    // Auto-scroll para acompanhar as últimas mensagens recebidas
    consoleDiv.scrollTop = consoleDiv.scrollHeight;
}


