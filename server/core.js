const { spawn } = require('child_process');
const path = require('path');

const CORE_PATH = path.resolve(__dirname, process.env.CORE_PATH || '../core/banca_core');

function eseguiCore(payload) {
  return new Promise((resolve, reject) => {
    const proc = spawn(CORE_PATH, [], { cwd: path.dirname(CORE_PATH) });
    let stdout = '';

    proc.stdout.on('data', d => stdout += d);
    proc.on('close', () => {
      if (!stdout.trim()) return reject(new Error('Nessuna risposta dal core'));
      try { resolve(JSON.parse(stdout.trim())); }
      catch { reject(new Error('Risposta core non valida: ' + stdout)); }
    });
    proc.on('error', err => reject(new Error('Impossibile avviare banca_core: ' + err.message)));

    proc.stdin.write(JSON.stringify(payload) + '\n');
    proc.stdin.end();
  });
}

module.exports = { eseguiCore };
