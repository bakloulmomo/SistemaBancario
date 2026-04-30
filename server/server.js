require('dotenv').config();
const express = require('express');
const { authMiddleware } = require('./middleware/auth');

const app = express();
app.use(express.json());
app.use(authMiddleware);

app.use('/api/auth',         require('./routes/auth'));
app.use('/api/conti',        require('./routes/conti'));
app.use('/api/transazioni',  require('./routes/transazioni'));
app.use('/api/utenti',       require('./routes/utenti'));
app.use('/api/notifiche',    require('./routes/notifiche'));
app.use('/api/chatbot',      require('./chatbot/routes'));

app.get('/api/health', (req, res) => res.json({ status: 'ok' }));

const PORT = process.env.PORT || 3000;
app.listen(PORT, () => console.log(`Server su http://localhost:${PORT}`));
