const { Router } = require('express');
const { eseguiCore } = require('../core');
const { requireAuth } = require('../middleware/auth');

const router = Router();
router.use(requireAuth);

router.post('/preleva', async (req, res) => {
  const { importo, descrizione = '' } = req.body;
  if (importo == null)
    return res.status(400).json({ status: 'error', message: 'importo richiesto' });
  try {
    const r = await eseguiCore({ cmd: 'preleva', token: req.token, importo: parseFloat(importo), descrizione });
    res.json(r);
  } catch (e) { res.status(500).json({ status: 'error', message: e.message }); }
});

router.post('/invia', async (req, res) => {
  const { iban_destinatario, importo, descrizione = '' } = req.body;
  if (!iban_destinatario || importo == null)
    return res.status(400).json({ status: 'error', message: 'iban_destinatario e importo richiesti' });
  try {
    const r = await eseguiCore({ cmd: 'invia', token: req.token, iban_destinatario, importo: parseFloat(importo), descrizione });
    res.json(r);
  } catch (e) { res.status(500).json({ status: 'error', message: e.message }); }
});

module.exports = router;
