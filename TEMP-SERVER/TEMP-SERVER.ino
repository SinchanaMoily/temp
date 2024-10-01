#include <DHT.h>
#include <WiFi.h>

const char* ssid = "workspace";
const char* password = "passcode";

#define DHTPIN 8
#define DHTTYPE DHT11

DHT dht(DHTPIN, DHTTYPE);

WiFiServer server(80);

void setup() {
  Serial.begin(9600);
  dht.begin();

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("Connected to WiFi");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  server.begin();
}

void handleRoot(WiFiClient client) {
  float humidity = dht.readHumidity();
  float temperature = dht.readTemperature();

  if (isnan(humidity) || isnan(temperature)) {
    humidity = 0;
    temperature = 0;
  }

  client.println("HTTP/1.1 200 OK");
  client.println("Content-type:text/html");
  client.println("Connection: close");
  client.println();

  client.println("<!DOCTYPE html><html>");
  client.println("<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">");
  client.println("<style>");
  client.println("body { font-family: 'Roboto', sans-serif; text-align: center; margin: 0; background-color: #f5f5f5; color: #333; }");
  client.println("h1 { color: #222; }");
  client.println(".container { display: flex; flex-direction: column; align-items: center; padding: 20px; }");

  // Circular gauge styling
  client.println(".circle-gauge { width: 90vw; height: 90vw; max-width: 300px; max-height: 300px; border-radius: 50%; position: relative; background: conic-gradient(#4caf50 0% 50%, #ff9800 50% 100%); border: 10px solid #ddd; box-shadow: 0 4px 6px rgba(0, 0, 0, 0.1); margin-bottom: 20px; }");
  client.println(".circle-gauge .gauge-fill { width: 100%; height: 100%; position: absolute; border-radius: 50%; clip: rect(0px, 300px, 150px, 0px); }");
  client.println(".circle-gauge .gauge-fill:before { content: ''; width: 100%; height: 100%; position: absolute; border-radius: 50%; background: radial-gradient(circle at center, #fff 0%, transparent 70%); }");
  client.println(".circle-gauge .gauge-text { position: absolute; top: 50%; left: 50%; transform: translate(-50%, -50%); font-size: 18px; color: #333; font-weight: bold; }");

  // Chart container styling
  client.println(".chart-container { width: 90%; max-width: 800px; margin-bottom: 20px; }");
  client.println("@media (max-width: 600px) { .chart-container { width: 100%; } }");

  client.println("@media (max-width: 600px) {");
  client.println("  .circle-gauge { width: 80vw; height: 80vw; max-width: 250px; max-height: 250px; }");
  client.println("  .circle-gauge .gauge-text { font-size: 16px; }");
  client.println("}");
  client.println("</style>");
  client.println("<script src=\"https://cdn.jsdelivr.net/npm/chart.js\"></script>");
  client.println("<script>");
  client.println("let tempChart, humidityChart;");
  client.println("const labels = [];");
  client.println("const tempData = [];");
  client.println("const humidityData = [];");
  client.println("function updateGauge(temperature, humidity) {");
  client.println("  const gauge = document.querySelector('.circle-gauge');");
  client.println("  const gaugeText = document.querySelector('.circle-gauge .gauge-text');");

  client.println("  // Calculate fill percentages");
  client.println("  const tempFill = Math.min(temperature / 50 * 100, 100);");
  client.println("  const humidityFill = Math.min(humidity / 100 * 100, 100);");

  client.println("  gauge.style.background = `conic-gradient(#4caf50 ${tempFill}%, #ff9800 ${tempFill}% ${humidityFill}%, #ddd ${humidityFill}% 100%)`;");
  client.println("  gaugeText.innerHTML = `Temp: ${temperature}&deg;C<br>Humidity: ${humidity}%`;");
  client.println("}");

  client.println("function updateCharts(temperature, humidity) {");
  client.println("  labels.push(new Date().toLocaleTimeString());");
  client.println("  tempData.push(temperature);");
  client.println("  humidityData.push(humidity);");
  client.println("  if (tempData.length > 10) {");
  client.println("    labels.shift();");
  client.println("    tempData.shift();");
  client.println("    humidityData.shift();");
  client.println("  }");
  client.println("  if (tempChart) {");
  client.println("    tempChart.data.labels = labels;");
  client.println("    tempChart.data.datasets[0].data = tempData;");
  client.println("    tempChart.update();");
  client.println("  } else {");
  client.println("    const ctx1 = document.getElementById('tempChart').getContext('2d');");
  client.println("    tempChart = new Chart(ctx1, { type: 'line', data: { labels: labels, datasets: [{ label: 'Temperature (°C)', data: tempData, borderColor: 'rgba(255, 99, 132, 1)', backgroundColor: 'rgba(255, 99, 132, 0.2)' }] }, options: { responsive: true, maintainAspectRatio: false } });");
  client.println("  }");
  client.println("  if (humidityChart) {");
  client.println("    humidityChart.data.labels = labels;");
  client.println("    humidityChart.data.datasets[0].data = humidityData;");
  client.println("    humidityChart.update();");
  client.println("  } else {");
  client.println("    const ctx2 = document.getElementById('humidityChart').getContext('2d');");
  client.println("    humidityChart = new Chart(ctx2, { type: 'line', data: { labels: labels, datasets: [{ label: 'Humidity (%)', data: humidityData, borderColor: 'rgba(54, 162, 235, 1)', backgroundColor: 'rgba(54, 162, 235, 0.2)' }] }, options: { responsive: true, maintainAspectRatio: false } });");
  client.println("  }");
  client.println("}");

  client.println("function fetchData() {");
  client.println("  fetch('/data').then(response => response.json()).then(data => {");
  client.println("    if (data.temperature !== undefined && data.humidity !== undefined) {");
  client.println("      updateGauge(data.temperature, data.humidity);");
  client.println("      updateCharts(data.temperature, data.humidity);");
  client.println("    }");
  client.println("  });");
  client.println("}");

  client.println("function autoRefresh() {");
  client.println("  fetchData();");
  client.println("  setTimeout(autoRefresh, 10000);");
  client.println("}");

  client.println("window.onload = autoRefresh;");
  client.println("</script>");
  client.println("</head>");
  client.println("<body>");
  client.println("<div class=\"container\">");
  client.println("<h1>Temperature and Humidity Dashboard</h1>");

  // Circular gauge for temperature and humidity
  client.println("<div class=\"circle-gauge\">");
  client.println("<div class=\"gauge-fill\"></div>");
  client.println("<div class=\"gauge-text\">Loading...</div>");
  client.println("</div>");

  // Chart for temperature and humidity
  client.println("<div class=\"chart-container\">");
  client.println("<canvas id=\"tempChart\"></canvas>");
  client.println("</div>");
  client.println("<div class=\"chart-container\">");
  client.println("<canvas id=\"humidityChart\"></canvas>");
  client.println("</div>");

  client.println("</div>");
  client.println("</body>");
  client.println("</html>");
}

void handleData(WiFiClient client) {
  float humidity = dht.readHumidity();
  float temperature = dht.readTemperature();

  if (isnan(humidity) || isnan(temperature)) {
    humidity = 0;
    temperature = 0;
  }

  client.println("HTTP/1.1 200 OK");
  client.println("Content-type: application/json");
  client.println("Connection: close");
  client.println();

  client.print("{\"temperature\":");
  client.print(temperature);
  client.print(",\"humidity\":");
  client.print(humidity);
  client.println("}");
}

void loop() {
  WiFiClient client = server.available();

  if (client) {
       Serial.println("New Client.");
    String currentLine = "";
    String request = "";

    while (client.connected()) {
      if (client.available()) {
        char c = client.read();
        request += c;
        if (c == '\n') {
          if (currentLine.length() == 0) {
            if (request.indexOf("GET /data") != -1) {
              handleData(client);
            } else {
              handleRoot(client);
            }
            break;
          } else {
            currentLine = "";
          }
        } else if (c != '\r') {
          currentLine += c;
        }
      }
    }

    client.stop();
    Serial.println("Client disconnected.");
    Serial.println("");
  }
}

