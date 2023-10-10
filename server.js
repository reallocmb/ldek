const express = require('express');
const path = require('path');

const app = express();
const port = 3000; // Du kannst einen beliebigen Port wählen

// Statische Dateien im "frontend" Verzeichnis servieren
app.use(express.static(path.join(__dirname, 'frontend')));

// Routen-Handler für den Pfad "/"
app.get('/', (req, res) => {
  // Die index.html-Datei im "frontend" Verzeichnis wird zurückgegeben
  res.sendFile(path.join(__dirname, 'frontend', 'index.html'));
});

// POST-Endpoint, um den String-Payload zu verarbeiten
app.post('/save', (req, res) => {
  const payload = req.body.payload;
  console.log('String-Payload vom Client:', payload);

  // Hier kannst du auf den Payload reagieren und eine Antwort senden
  // Zum Beispiel: res.send('POST-Anfrage erfolgreich verarbeitet');

  res.send('POST-Anfrage erfolgreich verarbeitet'); // Sendet eine einfache Antwort zurück
});

// Den Server starten
app.listen(port, () => {
  console.log(`Der Server läuft auf Port ${port}`);
});
