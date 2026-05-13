const { Router } = require('express');
const { eseguiCore } = require('../core');
const { requireAuth } = require('../middleware/auth');

const router = Router();
router.use(requireAuth);

router.get('/', async (req, res) => {
  try {
    const r = await eseguiCore({ cmd: 'lista_conti', username: req.username, password: req.password });
    res.json(r);
  } catch (e) { res.status(500).json({ status: 'error', message: e.message }); }
});

router.get('/:iban/estratto', async (req, res) => {
  try {
    const r = await eseguiCore({ cmd: 'estratto_conto', username: req.username, password: req.password, iban: req.params.iban });
    res.json(r);
  } catch (e) { res.status(500).json({ status: 'error', message: e.message }); }
});

module.exports = router;
