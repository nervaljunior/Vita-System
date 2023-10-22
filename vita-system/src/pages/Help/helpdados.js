import React from "react";
import erroinserir from '../../images/erroinserirdados.png'
import parar from '../../images/parar.png'
import inserirdados from '../../images/inserirdados.png'
import iniciar from '../../images/iniciar.png'

export const HelpDados = () => {

    return (
        <div id='helpdados'>
            <h2>Iniciar/parar recepção de dados</h2>
  
            <p>Para iniciar a recepção de dados clique no botão GRAFICO 1, GRAFICO 2 ou em ambos (demarcado em vermelho).</p>
                <p><em>OBS¹: É importante iniciar o sistema sempre pelo gráfico 1.</em></p>
                <img src={iniciar} alt="Imagem do Aplicativo" width="1000" height="600" />

            <p>Após isso, seguirá para uma aba onde será necessário colocar os dados da antena.</p>
            <img src={inserirdados} alt="Imagem do Aplicativo" width="1000" height="600" />
            <p><em>OBS²: Caso haja informações inválidas, apresentará a seguinte mensagem de erro. 
                Caso contrario, o grafico irá iniciar a leitura.
            </em></p>
            <img src={erroinserir} alt="Imagem do Aplicativo" width="1000" height="600"/>
               

            <p>E para parar a recepção de dados clique no botão PARAR 1 e PARAR 2 (demarcado em amarelo).</p>
            <img src={parar} alt="Imagem do Aplicativo" width="1000" height="600" />
            
            <footer>
                <p>&copy; 2023 SIRIS. Todos os direitos reservados.</p>
            </footer>
        </div>
    )
}