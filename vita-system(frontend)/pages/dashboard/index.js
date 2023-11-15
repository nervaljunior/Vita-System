import React, { useEffect, useState } from 'react';
import axios from 'axios';
import Chart from 'chart.js/auto'; 
import styles from './Dashboard.module.css';
import Topbar from '../../src/components/Topbar/Topbar';

const Dashboard = () => {
  const [mqttData, setMqttData] = useState(null);
  const [temperatureHistory, setTemperatureHistory] = useState([]);
  const [humidityHistory, setHumidityHistory] = useState([]);
  const [pressureHistory, setPressureHistory] = useState([]);
  const [altitudeHistory, setAltitudeHistory] = useState([]);
  const [selectedChart, setSelectedChart] = useState('temperature');

  useEffect(() => {
    const fetchDataRealTime = async () => {
      try {
        const response = await axios.get('http://localhost:8000/get_data');
        setMqttData(response.data);
        setTemperatureHistory((prevHistory) => [...prevHistory, response.data.temperature].slice(-10));
        setHumidityHistory((prevHistory) => [...prevHistory, response.data.humidity].slice(-10));
        setPressureHistory((prevHistory) => [...prevHistory, response.data.pressure].slice(-10));
        setAltitudeHistory((prevHistory) => [...prevHistory, response.data.altitude].slice(-10));
      } catch (error) {
        console.error('Error fetching MQTT data:', error);
      }
    };

    const fetchDataInterval = setInterval(fetchDataRealTime, 1000);

    return () => {
      clearInterval(fetchDataInterval);
    };
  }, []);

  useEffect(() => {
    const createChart = (id, label, data) => {
      const ctx = document.getElementById(id)?.getContext('2d');
      if (ctx) {
        const existingChart = Chart.getChart(ctx);
        if (existingChart) {
          existingChart.destroy();
        }

        new Chart(ctx, {
          type: 'line',
          data: {
            labels: Array.from({ length: 10 }, (_, i) => i + 1),
            datasets: [
              {
                label: label,
                data: data,
                borderColor: 'rgba(255, 99, 132, 1)',
                borderWidth: 1,
                fill: false,
              },
            ],
          },
          options: {
            scales: {
              x: {
                type: 'linear',
                position: 'bottom',
              },
              y: {
                beginAtZero: true,
              },
            },
          },
        });
      }
    };

    if (selectedChart === 'temperature') {
      createChart('historyChart', 'Temperature', temperatureHistory);
    } else if (selectedChart === 'humidity') {
      createChart('historyChart', 'Humidity', humidityHistory);
    } else if (selectedChart === 'pressure') {
      createChart('historyChart', 'Pressure', pressureHistory);
    } else if (selectedChart === 'altitude') {
      createChart('historyChart', 'Altitude', altitudeHistory);
    }
  }, [selectedChart, temperatureHistory, humidityHistory, pressureHistory, altitudeHistory]);

  const handleChartContainerClick = (chartType) => {
    setSelectedChart(chartType);
  };

  useEffect(() => {
    const fetchDataRealTime = async () => {
      try {
        const response = await axios.get('http://localhost:8000/get_data_real_time');
        const newTemperatureValue = response.data.temperature;


        setTemperatureHistory((prevHistory) => {
          const newHistory = [...prevHistory, newTemperatureValue];
          return newHistory.slice(-10); 
        });
      } catch (error) {
        console.error('Error fetching real-time data:', error);
      }
    };

    const fetchDataInterval = setInterval(fetchDataRealTime, 1000); 

 
    return () => {
      clearInterval(fetchDataInterval);
    };
  }, []); 


  return (
    <div className={styles.body}>
        <link
          href="https://fonts.googleapis.com/icon?family=Material+Symbols+Sharp"
          rel="stylesheet"
        />  
        <Topbar/>
        <div className={styles.container}>
          <aside>
            <div className={styles.top}>
              <div className={styles.logo}>
                {/* <img src="images/logo.png" alt="" /> */}
                <h2>{mqttData ? mqttData.name : ''}</h2>
              </div>
              <div className={styles.close} id={styles.closeBtn}>
              <span className={styles.materialSymbolsSharp}> close </span>
              </div>
            </div>
            <div className={styles.sidebar}>
              <a href="#" className={styles.active}>
              <span className={styles.materialSymbolsSharp}> dashboard </span>
                <h3>Dashboard</h3>
              </a>
            </div>
          </aside>
          <main>
            <h1>{mqttData ? mqttData.dashboardTitle : ''}</h1>
            <div className={styles.connectionStatus}>
              <h3>
                Connection Status: <span className={styles.status}>Disconnected</span>
              </h3>
            </div>
            <div className={styles.insights}>
            <div className={styles.chartButton} onClick={() => handleChartContainerClick('temperature')}>

                <div className={styles.middle}>
                  <div className={styles.left}>
                    <h3>Razão I:E</h3>
                    <h1 id={styles.temperature}></h1>
                  </div>
                  <div className={styles.icon}>
                    <span className={styles.materialSymbolsSharp}>🌡️</span>
                  </div>
                </div>
              </div>
              <div className={styles.chartButton} onClick={() => handleChartContainerClick('humidity')}>
                <div className={styles.middle}>
                  <div className={styles.left}>
                    <h3>Volume corrente</h3>
                    <h1 id={styles.humidity}></h1>
                  </div>
                  <div className={styles.icon}>
                    <span className={styles.materialSymbolsSharp}>💧</span>
                  </div>
                </div>
  
              </div>
              <div className={styles.chartButton} onClick={() => handleChartContainerClick('pressure')}>

                <div className={styles.middle}>
                  <div className={styles.left}>
                    <h3>Frequência</h3>
                    <h1 id={styles.pressure}></h1>
                  </div>
                  <div className={styles.icon}>
                    <span className={styles.materialSymbolsSharp}>📊</span>
                  </div>
                </div>
         
              </div>
              <div className={styles.chartButton} onClick={() => handleChartContainerClick('altitude')}>

                <div className={styles.middle}>
                  <div className={styles.left}>
                    <h3>Fluxo médio</h3>
                    <h1 id={styles.altitude}></h1>
                  </div>
                  <div className={styles.icon}>
                    <span className={styles.materialSymbolsSharp}>🏔️</span>
                  </div>
                </div>
              </div>
      
            </div>
            <div className={styles.histories}>
            <h2>Gráfico</h2>
            <div className={styles.historyCharts}>
            <canvas id="historyChart" className={styles.historyDivs}></canvas>
            </div>
          </div>
        </main>
        <div className={styles.right}>
          <div className={styles.top}>
            <button id={styles.menuBtn}>
              <span className={styles.materialSymbolsSharp}> menu </span>
            </button>
            <div className={styles.themeToggler}>
              <span
                className={`material-symbols-sharp ${
                  mqttData && mqttData.darkMode ? styles.active : ''
                }`}
              >
                light_mode
              </span>
              <span
                className={`material-symbols-sharp ${
                  mqttData && !mqttData.darkMode ? styles.active : ''
                }`}
              >
                dark_mode
              </span>
            </div>
          </div>
          <div className={styles.gaugeCharts}>
            <h2>Unidades</h2>
         
            <div className={styles.item}>
              <div id={styles.temperatureGauge}></div>
            </div>
            <div className={styles.item}>
              <div id={styles.humidityGauge}></div>
            </div>
            <div className={styles.item}>
              <div id={styles.pressureGauge}></div>
            </div>
            <div className={styles.item}>
              <div id={styles.altitudeGauge}></div>
            </div>
          </div>
        </div>
      </div>
      
    </div>
  );
};


export default Dashboard;
