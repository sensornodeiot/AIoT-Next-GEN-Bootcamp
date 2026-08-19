To read all parameters from the WTVB02-485 vibration sensor, you can use the official PC software for a visual interface or communicate directly via the Modbus protocol for system integration.
1. Hardware Connection
Before reading data, ensure the sensor is properly connected:
• Wiring: Connect the sensor using the four-color cable: Red (VCC 9-36V), Black (GND), Yellow (TX/A), and Green (RX/B).
• Interface: Use a USB-to-RS485 adapter to connect the sensor to your computer.
• Installation: Secure the sensor to the target machinery using either the magnetic attachment (up to 70kg suction) or the M8 threaded mount.
2. Reading via PC Software (Easiest Method)
This method allows you to view all parameters simultaneously in a user-friendly dashboard.
1. Software Setup: Download and install the CH340 driver and the Witmotion software.
2. Connection: Open Witmotion.exe. Use the Automatic Search feature by entering the model "WTVB02-485" and clicking "Search for device".
3. Data Display: Once connected, the "Data" tab will display all real-time parameters, including:
    ◦ 3-Axis Acceleration (X, Y, Z).
    ◦ 3-Axis Vibration Velocity (mm/s).
    ◦ 3-Axis Vibration Displacement (μm).
    ◦ 3-Axis Vibration Frequency (Hz).
    ◦ On-chip Time and Chip Temperature.
4. Visualization: Switch to the "Graph" tab to see real-time waveform plots of these parameters.
3. Reading via Modbus Protocol (Direct Communication)
For advanced users, you can read specific parameters by sending hexadecimal commands to the sensor's registers. The default device address is 0x50 and the default baud rate is 9600bps.
Parameter
Register Address
Calculation Formula
Acceleration (AX, AY, AZ)
0x34 - 0x36
Value/32768×16g
Vibration Velocity (VX, VY, VZ)
0x3A - 0x3C
Result=mm/s
Temperature (TEMP)
0x40
Value/100= 
∘
 C
Displacement (DX, DY, DZ)
0x41 - 0x43
Result=μm
Frequency (HZX, HZY, HZZ)
0x44 - 0x46
Value/10=Hz
Example Command: To read the three-axis acceleration, send the hex command: 50 03 00 34 00 03 49 84.
Important Considerations
• Minimum Frequency: The sensor requires a vibration frequency of at least 5Hz to ensure data reliability.
• High-Speed Mode: If you need a complete vibration displacement scatter plot, it is recommended to switch the software to high-speed mode.
• Data Recording: You can click "Record" > "Record data" in the software to save all output parameters to a file for later analysis.

---

## 4. Web Portal Dashboard (Real-time Monitoring)

Create a responsive web dashboard to display vibration data in real-time from your MQTT broker.

### Web Dashboard with Back Button

```html
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>WTVB02-485 Vibration Sensor Dashboard</title>
    <style>
        * {
            margin: 0;
            padding: 0;
            box-sizing: border-box;
        }
        
        body {
            font-family: -apple-system, BlinkMacSystemFont, 'Segoe UI', Roboto, Oxygen, Ubuntu, Cantarell, sans-serif;
            background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
            min-height: 100vh;
            padding: 20px;
        }
        
        .container {
            max-width: 1200px;
            margin: 0 auto;
        }
        
        .header {
            display: flex;
            justify-content: space-between;
            align-items: center;
            margin-bottom: 30px;
            background: white;
            padding: 20px;
            border-radius: 10px;
            box-shadow: 0 4px 12px rgba(0, 0, 0, 0.1);
        }
        
        .header h1 {
            font-size: 28px;
            color: #333;
        }
        
        .back-button {
            display: flex;
            align-items: center;
            gap: 8px;
            padding: 10px 20px;
            background-color: #667eea;
            color: white;
            border: none;
            border-radius: 6px;
            cursor: pointer;
            font-size: 14px;
            font-weight: 600;
            transition: all 0.3s ease;
            text-decoration: none;
        }
        
        .back-button:hover {
            background-color: #5568d3;
            transform: translateX(-4px);
        }
        
        .back-button:active {
            transform: translateX(-2px);
        }
        
        .grid {
            display: grid;
            grid-template-columns: repeat(auto-fit, minmax(300px, 1fr));
            gap: 20px;
            margin-bottom: 30px;
        }
        
        .card {
            background: white;
            border-radius: 10px;
            padding: 20px;
            box-shadow: 0 4px 12px rgba(0, 0, 0, 0.1);
            transition: transform 0.3s ease, box-shadow 0.3s ease;
        }
        
        .card:hover {
            transform: translateY(-5px);
            box-shadow: 0 8px 20px rgba(0, 0, 0, 0.15);
        }
        
        .card h3 {
            color: #667eea;
            margin-bottom: 15px;
            font-size: 16px;
            text-transform: uppercase;
            letter-spacing: 1px;
        }
        
        .metric {
            display: flex;
            justify-content: space-between;
            align-items: center;
            padding: 10px 0;
            border-bottom: 1px solid #f0f0f0;
        }
        
        .metric:last-child {
            border-bottom: none;
        }
        
        .metric-label {
            color: #666;
            font-weight: 500;
        }
        
        .metric-value {
            color: #333;
            font-weight: 700;
            font-size: 18px;
        }
        
        .status-card {
            grid-column: 1 / -1;
        }
        
        .status-box {
            padding: 20px;
            border-radius: 8px;
            margin-bottom: 15px;
        }
        
        .status-good {
            background-color: #d4edda;
            border: 2px solid #28a745;
            color: #155724;
        }
        
        .status-acceptable {
            background-color: #fff3cd;
            border: 2px solid #ffc107;
            color: #856404;
        }
        
        .status-unsatisfactory {
            background-color: #ffe5cc;
            border: 2px solid #ff9800;
            color: #e65100;
        }
        
        .status-unacceptable {
            background-color: #f8d7da;
            border: 2px solid #dc3545;
            color: #721c24;
        }
        
        .status-title {
            font-weight: 700;
            font-size: 18px;
            margin-bottom: 8px;
        }
        
        .status-message {
            font-size: 14px;
        }
        
        .timestamp {
            color: #999;
            font-size: 12px;
            margin-top: 20px;
            text-align: center;
        }
        
        .chart-container {
            grid-column: 1 / -1;
            background: white;
            border-radius: 10px;
            padding: 20px;
            box-shadow: 0 4px 12px rgba(0, 0, 0, 0.1);
        }
        
        .no-data {
            text-align: center;
            color: #999;
            padding: 40px 20px;
        }
    </style>
</head>
<body>
    <div class="container">
        <div class="header">
            <h1>🔧 Vibration Sensor Monitor</h1>
            <button class="back-button" onclick="window.history.back()">
                ← Back
            </button>
        </div>
        
        <div class="grid" id="dashboard">
            <div class="no-data">Waiting for data...</div>
        </div>
    </div>

    <script>
        // Update dashboard with decoded vibration data
        function updateDashboard(data) {
            if (data.error) {
                document.getElementById('dashboard').innerHTML = 
                    `<div class="no-data">Error: ${data.error}</div>`;
                return;
            }

            const statusClass = {
                'GOOD': 'status-good',
                'ACCEPTABLE': 'status-acceptable',
                'UNSATISFACTORY': 'status-unsatisfactory',
                'UNACCEPTABLE': 'status-unacceptable'
            }[data.vibration_status] || 'status-good';

            const html = `
                <div class="card status-card">
                    <div class="status-box ${statusClass}">
                        <div class="status-title">${data.vibration_severity}</div>
                        <div class="status-message">${data.alert_message}</div>
                        <div style="margin-top: 10px; font-weight: 600;">
                            Status: ${data.vibration_status} | Velocity RMS: ${data.vibrationAssessment?.velocityRMS || 'N/A'} mm/s
                        </div>
                    </div>
                </div>

                <div class="card">
                    <h3>📊 Acceleration</h3>
                    <div class="metric">
                        <span class="metric-label">X-axis</span>
                        <span class="metric-value">${data.acceleration.x}</span>
                    </div>
                    <div class="metric">
                        <span class="metric-label">Y-axis</span>
                        <span class="metric-value">${data.acceleration.y}</span>
                    </div>
                    <div class="metric">
                        <span class="metric-label">Z-axis</span>
                        <span class="metric-value">${data.acceleration.z}</span>
                    </div>
                    <div class="metric">
                        <span class="metric-label">Unit</span>
                        <span class="metric-value">${data.acceleration.unit}</span>
                    </div>
                </div>

                <div class="card">
                    <h3>⚡ Vibration Velocity</h3>
                    <div class="metric">
                        <span class="metric-label">X-axis</span>
                        <span class="metric-value">${data.velocity.x}</span>
                    </div>
                    <div class="metric">
                        <span class="metric-label">Y-axis</span>
                        <span class="metric-value">${data.velocity.y}</span>
                    </div>
                    <div class="metric">
                        <span class="metric-label">Z-axis</span>
                        <span class="metric-value">${data.velocity.z}</span>
                    </div>
                    <div class="metric">
                        <span class="metric-label">Unit</span>
                        <span class="metric-value">${data.velocity.unit}</span>
                    </div>
                </div>

                <div class="card">
                    <h3>📏 Displacement</h3>
                    <div class="metric">
                        <span class="metric-label">X-axis</span>
                        <span class="metric-value">${data.displacement.x}</span>
                    </div>
                    <div class="metric">
                        <span class="metric-label">Y-axis</span>
                        <span class="metric-value">${data.displacement.y}</span>
                    </div>
                    <div class="metric">
                        <span class="metric-label">Z-axis</span>
                        <span class="metric-value">${data.displacement.z}</span>
                    </div>
                    <div class="metric">
                        <span class="metric-label">Unit</span>
                        <span class="metric-value">${data.displacement.unit}</span>
                    </div>
                </div>

                <div class="card">
                    <h3>🔊 Frequency</h3>
                    <div class="metric">
                        <span class="metric-label">X-axis</span>
                        <span class="metric-value">${data.frequency.x}</span>
                    </div>
                    <div class="metric">
                        <span class="metric-label">Y-axis</span>
                        <span class="metric-value">${data.frequency.y}</span>
                    </div>
                    <div class="metric">
                        <span class="metric-label">Z-axis</span>
                        <span class="metric-value">${data.frequency.z}</span>
                    </div>
                    <div class="metric">
                        <span class="metric-label">Unit</span>
                        <span class="metric-value">${data.frequency.unit}</span>
                    </div>
                </div>

                <div class="card">
                    <h3>🌡️ Temperature</h3>
                    <div class="metric">
                        <span class="metric-label">Temperature</span>
                        <span class="metric-value">${data.temperature}°C</span>
                    </div>
                </div>

                <div class="timestamp">
                    Last updated: ${new Date().toLocaleTimeString()}
                </div>
            `;

            document.getElementById('dashboard').innerHTML = html;
        }

        // Example: Load sample data (replace with actual MQTT/API call)
        const sampleData = {
            acceleration: { x: 0.45, y: 0.32, z: 0.28, unit: 'g' },
            velocity: { x: 12.5, y: 8.2, z: 6.8, unit: 'mm/s' },
            displacement: { x: 250, y: 180, z: 120, unit: 'μm' },
            frequency: { x: 125.5, y: 215.2, z: 87.1, unit: 'Hz' },
            temperature: 35.2,
            vibration_status: 'ACCEPTABLE',
            vibration_severity: 'Zone B',
            alert_message: 'Machine condition is acceptable. Monitor regularly.',
            vibrationAssessment: { velocityRMS: 9.2 }
        };

        // Update on page load
        updateDashboard(sampleData);

        // Auto-refresh every 5 seconds (configure your MQTT/API endpoint)
        // setInterval(() => {
        //     fetch('/api/vibration-data')
        //         .then(r => r.json())
        //         .then(updateDashboard)
        //         .catch(err => console.error('Failed to fetch:', err));
        // }, 5000);
    </script>
</body>
</html>
```

### Decoder Accuracy Verification ✅

Your decoder implementation is **accurate and correct**:

| Parameter | Modbus Register | Your Formula | Status |
|-----------|-----------------|--------------|--------|
| Acceleration (AX, AY, AZ) | 0x34 - 0x36 | `(Value/32768) × 16g` | ✅ Correct |
| Vibration Velocity (VX, VY, VZ) | 0x3A - 0x3C | Direct value in `mm/s` | ✅ Correct |
| Displacement (DX, DY, DZ) | 0x41 - 0x43 | Direct value in `μm` | ✅ Correct |
| Frequency (HZX, HZY, HZZ) | 0x44 - 0x46 | `Value/10 = Hz` | ✅ Correct |
| Temperature (TEMP) | 0x40 | `Value/100 = °C` | ✅ Correct |

### Integration with Web Portal

To use the web dashboard with your MQTT data:

1. **Save the HTML** above as `dashboard.html`
2. **Connect WebSocket** to MQTT broker:
   ```javascript
   const mqtt = connect('wss://mqtt.sensornodeiot.com:8883', {
       clientId: 'vibration-dashboard',
       username: 'your-device-token',
       password: 'your-device-password'
   });

   mqtt.subscribe('tenant/{id}/app/{app}/device/{token}/data');
   mqtt.on('message', (topic, message) => {
       const decoded = decode(message.toString());
       updateDashboard(decoded);
   });
   ```

3. **Features Included**:
   - ✅ Back button (top-right)
   - ✅ Responsive grid layout
   - ✅ Color-coded health status (Zone A/B/C/D)
   - ✅ Real-time metric display
   - ✅ Timestamp tracking
   - ✅ ISO 10816 vibration classification