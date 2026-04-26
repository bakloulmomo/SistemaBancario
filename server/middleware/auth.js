function authMiddleware(req, res, next) {
  const header = req.headers['authorization'] || '';
  const token = header.startsWith('Bearer ') ? header.slice(7) : null;
  req.token = token;
  next();
}

function requireAuth(req, res, next) {
  if (!req.token) return res.status(401).json({ status: 'error', message: 'Non autenticato' });
  next();
}

module.exports = { authMiddleware, requireAuth };
