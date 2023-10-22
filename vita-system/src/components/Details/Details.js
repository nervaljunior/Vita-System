import React, { useState, useEffect } from 'react';
import ChartComponent from '../chartComponet/chartComponent';

function Details({ data, selectedCardName }) {
  const [chartData, setChartData] = useState({
    labels: [],
    values: Array(21).fill(0), // Inicialize a fila com 10 espaços preenchidos com zeros
  });

  useEffect(() => {
    const interval = setInterval(() => {
      // Obtenha a hora atual
      const now = new Date();
      const hours = now.getHours();
      const minutes = now.getMinutes();
      const seconds = now.getSeconds();

      try {
        // Use os dados recebidos do componente Geral (apiData) para preencher o gráfico
        const randomValue = data[selectedCardName] ;
        //const randomValue = Math.random() * 100;

        const newValues = [...chartData.values.slice(1), randomValue];

        const newLabels = [
          `${hours}:${minutes}:${seconds}`,
          `${hours}:${minutes}:${seconds + 1}`,
          `${hours}:${minutes}:${seconds + 2}`,
          `${hours}:${minutes}:${seconds + 3}`,
          `${hours}:${minutes}:${seconds + 4}`,
          `${hours}:${minutes}:${seconds + 5}`,
          `${hours}:${minutes}:${seconds + 6}`,
          `${hours}:${minutes}:${seconds + 7}`,
          `${hours}:${minutes}:${seconds + 8}`,
          `${hours}:${minutes}:${seconds + 9}`,
          `${hours}:${minutes}:${seconds + 10}`,
          `${hours}:${minutes}:${seconds + 11}`,
          `${hours}:${minutes}:${seconds + 12}`,
          `${hours}:${minutes}:${seconds + 13}`,
          `${hours}:${minutes}:${seconds + 14}`,
          `${hours}:${minutes}:${seconds + 15}`,
          `${hours}:${minutes}:${seconds + 16}`,
          `${hours}:${minutes}:${seconds + 17}`,
          `${hours}:${minutes}:${seconds + 18}`,
          `${hours}:${minutes}:${seconds + 19}`,
          `${hours}:${minutes}:${seconds + 20}`,
        ];

        setChartData({
          labels: newLabels,
          values: newValues,
        });
      } catch (error) {
        console.error('Erro ao buscar dados da API:', error);
      }
    }, 200);

    return () => clearInterval(interval);
  }, [chartData, data, selectedCardName]);

  return (
    <div className="details">
      <ChartComponent data={chartData} />
    </div>
  );
}

export default Details;
