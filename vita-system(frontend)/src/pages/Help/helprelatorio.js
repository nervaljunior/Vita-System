import React from "react";
import salvar from '../../images/salvarrelatorio.png'
import relatorio from '../../images/relatoriosalvo.png'

export const HelpRelatorio = () => {

    return (
        <div id='helpdados'>
            <h2>Gerar relatório</h2>
  
            <p>Após parar a recepção de dados, uma nova janela se abrirá informando se deseja
                gerar o relatório, aperte SIM para gerar e o relatorio, onde ele será exportado 
                para um arquivo .csv no diretorio C:/ .</p>
            
                <img src={salvar} alt="Imagem do Aplicativo" width="1000" height="600" />
                <img src={relatorio} alt="Imagem do Aplicativo" width="1000" height="600" />
            <footer>
                <p>&copy; 2023 SIRIS. Todos os direitos reservados.</p>
            </footer>
        </div>
    )
}