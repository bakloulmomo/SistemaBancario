const { Router } = require('express');
const { chiediGemini } = require('./gemini');

const router = Router();

router.post('/messaggio', async (req, res) => {
  const { messaggio, storico = [] } = req.body;
  if (!messaggio?.trim()) return res.status(400).json({ status: 'error', message: 'messaggio richiesto' });
  try {
    const risposta = await chiediGemini(messaggio, storico);
    res.json({ status: 'ok', risposta });
  } catch (e) {
    res.status(500).json({ status: 'error', message: e.message });
  }
});

module.exports = router;
