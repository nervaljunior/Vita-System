// ChartComponent.js
import React, { useEffect, useRef } from 'react';
import Chart from 'chart.js/auto';

function ChartComponent({ data }) {
  const chartRef = useRef(null);
  const chartInstance = useRef(null);

  useEffect(() => {
    const ctx = chartRef.current.getContext('2d');

    if (chartInstance.current) {
      chartInstance.current.destroy(); // Destrua o gráfico anterior, se existir
    }

    chartInstance.current = new Chart(ctx, {
      type: 'line', // Escolha o tipo de gráfico desejado (por exemplo, 'bar', 'line', 'pie', etc.)
      data: {
        labels: data.labels,
        datasets: [
          {
            label: 'Meu Gráfico', // aqui euquero colocar o nome da variavel 
            data: data.values,
            backgroundColor: 'rgba(75, 192, 192, 0.2)',
            borderColor: 'rgba(75, 192, 192, 1)',
            borderWidth: 1,
          },
        ],
      },
      options: {
        // Configurações adicionais do gráfico
      },
    });

    return () => {
      if (chartInstance.current) {
        chartInstance.current.destroy(); // Destrua o gráfico quando o componente for desmontado
      }
    };
  }, [data]);

  return <canvas ref={chartRef} />;
}

export default ChartComponent;
