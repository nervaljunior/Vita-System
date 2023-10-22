import React, { useState, useEffect } from 'react';

export const Cronometro = ({iniciarCronometro, pararCronometro}) => {
  const [tempo, setTempo] = useState(0);

  useEffect(() => {
    let interval;

    if (iniciarCronometro && !pararCronometro) {
      setTempo(0); 
      interval = setInterval(() => {
        setTempo(tempo => tempo + 1);
      }, 1000);
    }

    return () => {
      clearInterval(interval);
    };
  }, [iniciarCronometro, pararCronometro]);

  const formatarTempo = tempo => {
    const horas = Math.floor(tempo / 3600);
    const minutos = Math.floor((tempo % 3600) / 60);
    const segundos = tempo % 60;

    const formatado =
      horas.toString().padStart(2, '0') +
      ':' +
      minutos.toString().padStart(2, '0') +
      ':' +
      segundos.toString().padStart(2, '0');

    return formatado;
  };

  return (
      <div id="cronometro">{formatarTempo(tempo)}</div>
  );
};

