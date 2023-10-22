import React, { useState, useEffect } from 'react';
import '../../pages/func/func.css'
import { BsFillCircleFill} from 'react-icons/bs';
import {
  Chart as ChartJS,
  CategoryScale,
  LinearScale,
  PointElement,
  LineElement,
  Title,
  Tooltip,
  Legend,
  defaults,
} from 'chart.js';
import { Line } from 'react-chartjs-2';
import axios from 'axios';

ChartJS.register(
  CategoryScale,
  LinearScale,
  PointElement,
  LineElement,
  Title,
  Tooltip,
  Legend
);

//grafico1
export function Grafico({iniciar}) {
  const [labels, setLabels] = useState([]);
  const [azimute, setAzimute] = useState([]);
  const [elevacao, setElevacao] = useState([]);
  const [graficoContainerWidth, setGraficoContainerWidth] = useState('');


  //funcao para atualizar o grafico
  const atualizarGrafico = () => {
      axios.get('http://localhost:4000/buffer')
        .then(response => {
          const { antena1: apiData } = response.data;
          const azimute = apiData[0]
          const elevacao = apiData[1]
          const time = apiData[2]
          setLabels(time);
          setAzimute(azimute);
          setElevacao(elevacao);
        })
        .catch(error => {
          console.error('Error:', error);
        });
  };

  useEffect(() => {
    if (iniciar==true) {
      atualizarGrafico();
      const interval = setInterval(atualizarGrafico, 500);
      return () => {
        clearInterval(interval);
      };
    }
  }, [iniciar]);

  //criando grafico
  const chartData = {
    labels,
    datasets: [
      {
        label: 'Azimute',
        data: azimute,
        borderColor: 'rgb(255, 99, 132)',
        backgroundColor: 'rgba(255, 99, 132, 0.5)',
      },
      {
        label: 'Elevacao',
        data: elevacao,
        borderColor: 'rgb(53, 162, 235)',
        backgroundColor: 'rgba(53, 162, 235, 0.5)',
      },
    ],
  };

  useEffect(() => {
    if (labels && labels.length > 7) {
      const newWidth = 700 + (labels.length - 7) * 30;
      setGraficoContainerWidth(`${newWidth}px`);
    }
  }, [labels]);
  

   return (
      <div className='graficoContainer' style={{ width: graficoContainerWidth, height: '700px'}}>
        <Line className='grafico' data={chartData} options={{ maintainAspectRatio: false }} />
      </div>
  );
}


//grafico2
export function Grafico2({iniciar}) {
  const [labels, setLabels] = useState([]);
  const [azimute, setAzimute] = useState([]);
  const [elevacao, setElevacao] = useState([]);
  const [graficoContainerWidth, setGraficoContainerWidth] = useState('');


  //funcao para atualizar o grafico
  const atualizarGrafico = () => {
      axios.get('http://localhost:4000/buffer')
        .then(response => {
          const { antena2: apiData } = response.data;
          const azimute = apiData[0]
          const elevacao = apiData[1]
          const time = apiData[2]
          setLabels(time);
          setAzimute(azimute);
          setElevacao(elevacao);
        })
        .catch(error => {
          console.error('Error:', error);
        });
  };

  useEffect(() => {
    if (iniciar==true) {
      atualizarGrafico();
      const interval = setInterval(atualizarGrafico, 500);
      return () => {
        clearInterval(interval);
      };
    }
  }, [iniciar]);

  //criando grafico
  const chartData = {
    labels,
    datasets: [
      {
        label: 'Azimute',
        data: azimute,
        borderColor: 'rgb(255, 99, 132)',
        backgroundColor: 'rgba(255, 99, 132, 0.5)',
      },
      {
        label: 'Elevacao',
        data: elevacao,
        borderColor: 'rgb(53, 162, 235)',
        backgroundColor: 'rgba(53, 162, 235, 0.5)',
      },
    ],
  };

  useEffect(() => {
    if (labels && labels.length > 7) {
      const newWidth = 700 + (labels.length - 7) * 30;
      setGraficoContainerWidth(`${newWidth}px`);
    }
  }, [labels]);

   return (
      <div className='graficoContainer' style={{ width: graficoContainerWidth, height: '700px'}}>
        <Line className='grafico' data={chartData} options={{ maintainAspectRatio: false }} />
      </div>
  );
}



//exportando componente grafico
export default Grafico;
