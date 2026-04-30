const https = require('https');

const GEMINI_API_KEY = process.env.GEMINI_API_KEY;
const GEMINI_MODEL   = 'gemini-2.5-flash';
const GEMINI_URL     = `https://generativelanguage.googleapis.com/v1beta/models/${GEMINI_MODEL}:generateContent?key=${GEMINI_API_KEY}`;

const SYSTEM_PROMPT = `Sei un assistente bancario professionale per PayByte.
Aiuta l'utente con domande su: trasferimenti, saldi, conti, operazioni bancarie, sicurezza.
Rispondi in italiano, in modo conciso e professionale.
Non fornire consigli finanziari specifici. Non accedere a dati reali dell'utente.`;

async function chiediGemini(messaggio, storico = []) {
  const contents = [
    ...storico.map(m => ({ role: m.ruolo === 'user' ? 'user' : 'model', parts: [{ text: m.testo }] })),
    { role: 'user', parts: [{ text: messaggio }] }
  ];

  const body = JSON.stringify({
    system_instruction: { parts: [{ text: SYSTEM_PROMPT }] },
    contents
  });

  return new Promise((resolve, reject) => {
    const req = https.request(GEMINI_URL, {
      method: 'POST',
      headers: { 'Content-Type': 'application/json', 'Content-Length': Buffer.byteLength(body) }
    }, res => {
      let data = '';
      res.on('data', d => data += d);
      res.on('end', () => {
        try {
          const json = JSON.parse(data);
          const text = json?.candidates?.[0]?.content?.parts?.[0]?.text;
          if (!text) return reject(new Error('Risposta Gemini vuota'));
          resolve(text);
        } catch { reject(new Error('Risposta Gemini non valida')); }
      });
    });
    req.on('error', reject);
    req.write(body);
    req.end();
  });
}

module.exports = { chiediGemini };
