const { Router } = require('express');
const { eseguiCore } = require('../core');
const { requireAuth } = require('../middleware/auth');

const router = Router();
router.use(requireAuth);

router.put('/profilo', async (req, res) => {
  const { nome, cognome } = req.body;
  if (!nome || !cognome)
    return res.status(400).json({ status: 'error', message: 'nome e cognome richiesti' });
  try {
    const r = await eseguiCore({ cmd: 'aggiorna_profilo', token: req.token, nome, cognome });
    res.json(r);
  } catch (e) { res.status(500).json({ status: 'error', message: e.message }); }
});

router.put('/password', async (req, res) => {
  const { vecchia_password, nuova_password } = req.body;
  if (!vecchia_password || !nuova_password)
    return res.status(400).json({ status: 'error', message: 'vecchia_password e nuova_password richiesti' });
  try {
    const r = await eseguiCore({ cmd: 'cambia_password', token: req.token, vecchia_password, nuova_password });
    res.json(r);
  } catch (e) { res.status(500).json({ status: 'error', message: e.message }); }
});

router.get('/cerca', async (req, res) => {
  const query = req.query.q || '';
  try {
    const r = await eseguiCore({ cmd: 'cerca_utenti', token: req.token, query });
    res.json(r);
  } catch (e) { res.status(500).json({ status: 'error', message: e.message }); }
});

router.delete('/account', async (req, res) => {
  const { password } = req.body;
  if (!password)
    return res.status(400).json({ status: 'error', message: 'password richiesta per conferma' });
  try {
    const r = await eseguiCore({ cmd: 'elimina_account', token: req.token, password });
    res.json(r);
  } catch (e) { res.status(500).json({ status: 'error', message: e.message }); }
});

module.exports = router;
