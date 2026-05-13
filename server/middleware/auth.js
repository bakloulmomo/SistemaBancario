const crypto = require('crypto');

const tokenStore = new Map(); // token → { username, password }

function storeToken(username, password) {
  const token = crypto.randomBytes(32).toString('hex');
  tokenStore.set(token, { username, password });
  return token;
}

function removeToken(token) {
  tokenStore.delete(token);
}

function aggiornaPw(token, nuovaPassword) {
  const cred = tokenStore.get(token);
  if (cred) cred.password = nuovaPassword;
}

function authMiddleware(req, res, next) {
  const header = req.headers['authorization'] || '';
  const token = header.startsWith('Bearer ') ? header.slice(7) : null;
  req.token = token;
  if (token && tokenStore.has(token)) {
    const cred = tokenStore.get(token);
    req.username = cred.username;
    req.password = cred.password;
  }
  next();
}

function requireAuth(req, res, next) {
  if (!req.username) return res.status(401).json({ status: 'error', message: 'Non autenticato' });
  next();
}

module.exports = { authMiddleware, requireAuth, storeToken, removeToken, aggiornaPw };
