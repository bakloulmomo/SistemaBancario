const { Router } = require('express');
const { eseguiCore } = require('../core');
const { requireAuth, storeToken, removeToken } = require('../middleware/auth');

const router = Router();

router.post('/registra', async (req, res) => {
  const { username, nome, cognome, eta, password, saldo_iniziale = 0 } = req.body || {};
  if (!nome || !cognome || !eta || !password)
    return res.status(400).json({ status: 'error', message: 'nome, cognome, eta e password richiesti' });
  if (parseInt(eta) < 18)
    return res.status(400).json({ status: 'error', message: 'età minima 18 anni' });
  try {
    const payload = { cmd: 'registra', nome, cognome, eta: parseInt(eta), password, saldo_iniziale: parseFloat(saldo_iniziale) || 0 };
    if (username) payload.username = username;
    const r = await eseguiCore(payload);
    if (r.status !== 'ok') return res.status(400).json(r);
    const token = storeToken(r.data.username || username, password);
    res.json({ ...r, data: { ...r.data, token } });
  } catch (e) {
    res.status(500).json({ status: 'error', message: e.message });
  }
});

router.post('/login', async (req, res) => {
  const { username, password } = req.body || {};
  if (!username || !password)
    return res.status(400).json({ status: 'error', message: 'username e password richiesti' });
  try {
    const r = await eseguiCore({ cmd: 'login', username, password });
    if (r.status !== 'ok') return res.status(401).json(r);
    const token = storeToken(username, password);
    res.json({ ...r, data: { ...r.data, token } });
  } catch (e) {
    res.status(500).json({ status: 'error', message: e.message });
  }
});

router.post('/logout', requireAuth, (req, res) => {
  removeToken(req.token);
  res.json({ status: 'ok' });
});

router.get('/profilo', requireAuth, async (req, res) => {
  try {
    const r = await eseguiCore({ cmd: 'profilo', username: req.username, password: req.password });
    res.json(r);
  } catch (e) {
    res.status(500).json({ status: 'error', message: e.message });
  }
});

module.exports = router;
