const jwt = require('jsonwebtoken');

const secret = 'eKtdZrQtZhGrFXU1nu1FBne6uKK2O4uNVr+fgIlpBQwi/xazh1A/zf9gGS2IuIiuPezsBWww4/9mBllKQEB3Pw==';
const token = jwt.sign({
  role: 'authenticated',
  email: 'test@example.com',
  sub: '1234567890'
}, Buffer.from(secret, 'base64'), { expiresIn: '1h' });

fetch('http://localhost:8080/api/history/sensors', {
  headers: {
    'Authorization': `Bearer ${token}`
  }
})
.then(async res => {
  console.log('Status:', res.status);
  const text = await res.text();
  console.log('Body:', text);
})
.catch(err => {
  console.error('Fetch error:', err);
});
