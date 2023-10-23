import React, {useState, useEffect} from "react";
import axios from 'axios'
import {BsFillCircleFill} from 'react-icons/bs'
import './mensagem.css'


//verificando se ja esta recebendo dados grafico 2 e mostrando mensagem
export const Mensagem = ({ iniciar }) => {
    const [recebendo, setRecebendo] = useState();
    let tamanho;
  
    const recebendoDados = () => {
        axios.get('http://localhost:4000/buffer')
          .then(response => {
            const { antena1: apiData } = response.data;
            const azimute = apiData[0];
            tamanho = azimute.length - 1;
            if(azimute[tamanho] == 0) {
              setRecebendo(0)
            }
            else if(azimute[tamanho] !== 0) {
              setRecebendo(1)
            }
          })
          .catch(error => {
            console.error('Error:', error);
            setRecebendo(3);
          });
    };
  
    useEffect(() => {
      if (iniciar==true) {
        recebendoDados();
        const interval = setInterval(recebendoDados, 500);
        return () => {
          clearInterval(interval);
        };
      } else if(iniciar == false) {
        setRecebendo(3);
      }
    }, [iniciar]);
  
    return (
      <div>
        {recebendo === 0 && <div id='recepcao'><BsFillCircleFill />Aguardando Dados</div>}
        {recebendo === 1 && <div id='recebendo'>Recebendo Dados...</div>}
      </div>
    );
  };



  //verificando se ja esta recebendo dados grafico 1 e mostrando mensagem
export const Mensagem2 = ({ iniciar }) => {
    const [recebendo, setRecebendo] = useState();
    let tamanho;
  
    const recebendoDados = () => {
        axios.get('http://localhost:4000/buffer')
          .then(response => {
            const { antena2: apiData } = response.data;
            const azimute = apiData[0];
            tamanho = azimute.length - 1;
            if(azimute[tamanho] == 0) {
              setRecebendo(0)
            }
            else if(azimute[tamanho] !== 0) {
              setRecebendo(1)
            }
          })
          .catch(error => {
            console.error('Error:', error);
            setRecebendo(3);
          });
    };
  
    useEffect(() => {
      if (iniciar==true) {
        recebendoDados();
        const interval = setInterval(recebendoDados, 500);
        return () => {
          clearInterval(interval);
        };
      } else if(iniciar == false) {
        setRecebendo(3);
      }
    }, [iniciar]);
  
    return (
      <div>
        {recebendo === 0 && <div id='recepcao2'><BsFillCircleFill />Aguardando Dados</div>}
        {recebendo === 1 && <div id='recebendo2'>Recebendo Dados...</div>}
      </div>
    );
  };