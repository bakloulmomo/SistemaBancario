const { Router } = require('express');
const { eseguiCore } = require('../core');
const { requireAuth } = require('../middleware/auth');

const router = Router();
router.use(requireAuth);

router.get('/', async (req, res) => {
  try {
    const r = await eseguiCore({ cmd: 'notifiche', token: req.token });
    res.json(r);
  } catch (e) { res.status(500).json({ status: 'error', message: e.message }); }
});

module.exports = router;
