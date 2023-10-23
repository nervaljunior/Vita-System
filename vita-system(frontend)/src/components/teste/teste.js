import React, { useState, useEffect } from 'react';
import { Line } from 'react-chartjs-2';
import 'bootstrap/dist/css/bootstrap.min.css';

const SensorData = () => {
  const [chartData, setChartData] = useState({});

  useEffect(() => {
    // Gere dados aleatórios para o exemplo
    const generateRandomData = () => {
      const labels = [];
      const Volume_corrente = [];
      const Razao_IE = [];
      const Frequencia = [];
      const Fluxo_medio = [];

      for (let i = 1; i <= 10; i++) {
        labels.push(`Data ${i}`);
        Volume_corrente.push(Math.random() * 100);
        Razao_IE.push(Math.random() * 50);
        Frequencia.push(Math.random() * 60);
        Fluxo_medio.push(Math.random() * 40);
      }

      return { labels, Volume_corrente, Razao_IE, Frequencia, Fluxo_medio };
    };

    const randomData = generateRandomData();

    setChartData({
      labels: randomData.labels,
      datasets: [
        {
          label: 'Volume_corrente',
          data: randomData.Volume_corrente,
          borderColor: 'rgba(75, 192, 192, 1)',
          backgroundColor: 'rgba(75, 192, 192, 0.2)',
        },
        {
          label: 'Razão_IE',
          data: randomData.Razao_IE,
          borderColor: 'rgba(255, 99, 132, 1)',
          backgroundColor: 'rgba(255, 99, 132, 0.2)',
        },
        {
          label: 'Frequência',
          data: randomData.Frequencia,
          borderColor: 'rgba(192, 192, 75, 1)',
          backgroundColor: 'rgba(192, 192, 75, 0.2)',
        },
        {
          label: 'Fluxo_medio',
          data: randomData.Fluxo_medio,
          borderColor: 'rgba(192, 75, 75, 1)',
          backgroundColor: 'rgba(192, 75, 75, 0.2)',
        },
      ],
    });
  }, []);


  

  return (
    <div className="container">
      <div className="row">
        <div className="col-md-12">
        {chartData && chartData.labels && chartData.datasets && chartData.datasets.length > 0 && (
  <Line data={chartData} options={{}} />
)}

        </div>
      </div>
    </div>
  );
};

export default SensorData;
