import React, { useState, useEffect } from 'react';
import { Line } from 'react-chartjs-2';
import axios from 'axios';
import 'bootstrap/dist/css/bootstrap.min.css';

const SensorData = () => {
  const [chartData, setChartData] = useState({
    labels: [],
    datasets: [
      {
        label: 'Volume_corrente',
        data: [],
        borderColor: 'rgba(75, 192, 192, 1)',
        backgroundColor: 'rgba(75, 192, 192, 0.2)',
      },
      {
        label: 'Razao_IE',
        data: [],
        borderColor: 'rgba(255, 99, 132, 1)',
        backgroundColor: 'rgba(255, 99, 132, 0.2)',
      },
      {
        label: 'Frequencia',
        data: [],
        borderColor: 'rgba(192, 192, 75, 1)',
        backgroundColor: 'rgba(192, 192, 75, 0.2)',
      },
      {
        label: 'Fluxo_medio',
        data: [],
        borderColor: 'rgba(192, 75, 75, 1)',
        backgroundColor: 'rgba(192, 75, 75, 0.2)',
      },
    ],
  });

  const updateChartData = (newData) => {
    setChartData((prevData) => {
      const newLabels = [...prevData.labels, new Date().toLocaleTimeString()];
      if (newLabels.length > 10) {
        newLabels.shift();
      }

      const newDatasets = prevData.datasets.map((dataset) => {
        return {
          ...dataset,
          data: [...dataset.data, newData[dataset.label.replace('_', ' ')]] // Atualiza os dados do dataset correspondente
        };
      });

      return {
        labels: newLabels,
        datasets: newDatasets,
      };
    });
  };

  useEffect(() => {
    const fetchDataFromApi = async () => {
      try {
        const response = await axios.get('http://localhost:7777/get_random_data');
        const data = response.data;

        updateChartData(data);
      } catch (error) {
        console.error('Erro ao buscar dados da API:', error);
      }
    };

    fetchDataFromApi(); // Chame a função para buscar dados da API

    const interval = setInterval(fetchDataFromApi, 1000); // Atualize os dados a cada segundo

    return () => clearInterval(interval);
  }, []);

  return (
    <div className="grafico">
      <div className="row">
        <div className="col-md-12">
          <Line data={chartData} options={{}} />
        </div>
      </div>
    </div>
  );
};

export default SensorData;
