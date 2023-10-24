import React, { useState, useEffect } from 'react';
import Details from '../Details/Details';
import Card from '../Card/Card';
import axios from "axios";



function Geral() {
  const [selectedCardName, setSelectedCardName] = useState(null);
  const [apiData, setApiData] = useState({
    Volume_corrente: 0,
    Razão_IE: 0,
    Frequência: 0,
    Fluxo_médio: 0,
  });

  const handleCardClick = (cardName) => {
    setSelectedCardName(cardName);
  };

  useEffect(() => {
    const fetchDataFromApi = async () => {
      try {
        const response = await axios.get('http://localhost:7777/get_random_data');
        const data = response.data;
  
        // Atualize os valores da API
        setApiData(data);
      } catch (error) {
        console.error('Erro ao buscar dados da API:', error);
      }
    };
  
    fetchDataFromApi();
  
    const interval = setInterval(() => {
      fetchDataFromApi();
    }, 200);
  
    return () => clearInterval(interval);
  }, []);

  return (
    <div className=''>

      <div className="button-container">
        <button
          className={selectedCardName === 'Volume_corrente' ? 'selected' : 'unselected'}
          onClick={() => handleCardClick('Volume_corrente')}
        >
          <Card
            number={`${apiData.Volume_corrente !== undefined ? apiData.Volume_corrente.toFixed(2) : 'N/A'} mL`}
            cardName="Volume corrente"
            iconName="eye-outline"
          />
        </button>
        <button
          className={selectedCardName === 'Razão_IE' ? 'selected' : 'unselected'}
          onClick={() => handleCardClick('Razão_IE')}
        >
          <Card
            number={`1 : ${apiData.Razão_IE !== undefined ? apiData.Razão_IE.toFixed(2) : 'N/A'}`}
            cardName="Razão I:E"
            iconName="bar-chart-outline"
          />
        </button>
        <button
          className={selectedCardName === 'Frequência' ? 'selected' : 'unselected'}
          onClick={() => handleCardClick('Frequência')}
        >
          <Card
            number={`${apiData.Frequência !== undefined ? apiData.Frequência.toFixed(2) : 'N/A'} RPM`}
            cardName="Frequência"
            iconName="fitness-outline"
          />
        </button>
        <button
          className={selectedCardName === 'Fluxo_médio' ? 'selected' : 'unselected'}
          onClick={() => handleCardClick('Fluxo_médio')}
        >
          <Card
            number={`${apiData.Fluxo_médio !== undefined ? apiData.Fluxo_médio.toFixed(2) : 'N/A'} m³/min`}
            cardName="Fluxo médio"
            iconName="chatbubbles-outline"
          />
        </button>
      </div>
{/*       <div className='grafico'>

        <Details data={apiData} selectedCardName={selectedCardName} />
      </div> */}
    </div>
  );
}

export default Geral;
