import React from "react";
import './func.css'
import {BiCaretRight, BiSquare} from 'react-icons/bi'
import { AiOutlineClose } from 'react-icons/ai'
import { useAuth } from '../../Auth/context'
import { useState, useEffect } from "react";
import { ConfigAntena } from "../../components/ModalAntena/configAntena";
import { Grafico, Grafico2 } from "../../components/Grafico/grafico";
import { Mensagem, Mensagem2 } from "../../components/mensagem/mensagem";
import { Cronometro } from "../../components/Cronometro/cronometro";
import axios from "axios";
import { Save } from "../../components/save/save";
import { Salvo } from "../../components/salvo/salvo";

export const Funcionario = () => { 
    
    //VARIAVEIS PARA FAZER LOGOUT
    const login = useAuth();    
    const logout = () => login.logout();

    //VARIAVEL PARA ABRIR ABA DA CONFIG ANTEAN
    const [openConfigAntena, setOpenConfigAntena] = useState(false)
    const [isAntena, setIsAntena] = useState()

    //VARIAVEL PARA ABRIR ABA DE SALVAR RELATORIO
    const [openSave, setOpenSave] = useState(false)
    const [openSalvo, setOpenSalvo] = useState(false)

    //VARIAVEIS PARA INICIAR GRAFICOS
    const [gerarGrafico1, setGerarGrafico1] = useState(false);
    const [gerarGrafico2, setGerarGrafico2] = useState(false);

    //FUNCOES PARA PARAR O GRAFICO
    const pararGrafico1  = () => {
        axios.post("http://localhost:4000/configantena/stop1", {
            stop: true
        });
        setIsAntena("antena1")
        setGerarGrafico1(false); 
        setOpenSave(true)
        if(gerarGrafico2==false) {
            paraCronometro()
        }};
    
    const pararGrafico2  = () => {
        axios.post("http://localhost:4000/configantena/stop2", {
            stop: true
        });
        setIsAntena("antena2")
        setGerarGrafico2(false); 
        setOpenSave(true)
        if(gerarGrafico1==false) {
            paraCronometro()
        }};
        

    //CONFIGURANDO CRONOMETRO
    const [iniciarCronometro, setIniciarCronometro] = useState(false);
    const [pararCronometro, setPararCronometro] = useState(false);
    
    const paraCronometro = () => {
        setIniciarCronometro(false); 
        setPararCronometro(true);};

    //FUNCAO PARA IR PARA ABA DE HELP
    const ajuda = () => {
        window.open('/Help')
        window.focus()
    }

    //CONFIGURANDO ALERTA
    const [corAlerta, setCorAlerta] = React.useState('');
    const [alertando, setAlertando] = React.useState(false);
    const [corAlerta2, setCorAlerta2] = React.useState('');
    const [alertando2, setAlertando2] = React.useState(false);

    //ALERTA 1
    useEffect(() => {
    let interval;
    if (gerarGrafico1) {
        interval = setInterval(() => {
        axios.get('http://localhost:4000/configantena')
            .then(response => {
            const { anomalia1: apiData } = response.data;
            setAlertando(apiData);
            if(apiData == true) {
                clearInterval(interval);
            }
            })
            .catch(error => {
            console.error('Error:', error);
            });
            }, 50);
        }

        return () => {
            clearInterval(interval);
        };
        }, [gerarGrafico1]);
    

    useEffect(() => {
        let intervalId;
        if (alertando) {
          intervalId = setInterval(() => {
            setCorAlerta((corAnterior) =>
              corAnterior === ''
                ? 'rgba(255, 0, 0, 0.5)' // Alterne para outra cor desejada
                : '' // Volte para a cor original
            );
          }, 1000);
        } else if (!alertando) {
            intervalId = setInterval(() => {
                setCorAlerta('');
              }, 1000);
        }
    
        return () => {
          clearInterval(intervalId);
        };
      }, [alertando]);

    
    //ALERTA 2
    useEffect(() => {
    let intervalo2;
    if (gerarGrafico2) {
        intervalo2 = setInterval(() => {
        axios.get('http://localhost:4000/configantena')
            .then(response => {
            const { anomalia2: apiData } = response.data;
            setAlertando2(apiData);
            if(apiData == true) {
                clearInterval(intervalo2);
            }
            })
            .catch(error => {
            console.error('Error:', error);
            });
            }, 50);
        }

        return () => {
            clearInterval(intervalo2);
        };
        }, [gerarGrafico2]);
    

    useEffect(() => {
        let interval;
        if (alertando2) {
          interval = setInterval(() => {
            setCorAlerta2((corAnterior) =>
              corAnterior === ''
                ? 'rgba(255, 0, 0, 0.5)' // Alterne para outra cor desejada
                : '' // Volte para a cor original
            );
          }, 1000);
        } else if (!alertando) {
            interval = setInterval(() => {
                setCorAlerta2('');
              }, 1000);
        }
    
        return () => {
          clearInterval(interval);
        };
      }, [alertando2]);

      const pararanomalia = () => {setAlertando(false)}
      const pararanomalia2 = () => {setAlertando2(false)}

    return (
        <div className="paginaFuncionario">
            <div id="pagefunc" className="boxfunc">
                <button type="submit" class={`botao ${gerarGrafico1 ? 'active' : ''}`} onClick={() => {
                    setIsAntena("antena1"); 
                    setOpenConfigAntena(true)}}>
                <span><BiCaretRight/> GRÁFICO 1</span></button>
                
                <button type="submit" class='botaoStop' onClick={pararGrafico1}>
                <span><BiSquare/> PARAR 1</span></button>
                
                <button type="submit" class='botao' onClick={() => {
                    setIsAntena("antena2"); 
                    setOpenConfigAntena(true)}}>
                <span><BiCaretRight/> GRÁFICO 2</span></button>
                
                <button type="submit" class='botaoStop' onClick={pararGrafico2}>
                <span><BiSquare/> PARAR 2</span></button>
                
                <button id='help' type="submit" class='botao' onClick={ajuda}>HELP</button>
                
                <button id='logoutfunc' type="submit" onClick={logout}>SAIR</button>
            </div>
            <div>

                <div className="boxGrafico1" style={{ backgroundColor: corAlerta }}>
                    {alertando && <button id='pararanomalia'
                     onClick={pararanomalia}><AiOutlineClose /></button>}
                    <span>GRÁFICO 1</span>
                    <Grafico iniciar={gerarGrafico1} />                 
                </div>

                <div className="boxGrafico2" style={{ backgroundColor: corAlerta2 }}>
                    {alertando2 && <button id='pararanomalia2'
                    onClick={pararanomalia2}><AiOutlineClose /></button>}
                    <Grafico2 iniciar={gerarGrafico2} />
                    <span>GRÁFICO 2</span>
                
                </div>

                <div id='boxcronometro' > <Cronometro iniciarCronometro={iniciarCronometro} pararCronometro={pararCronometro}/> </div>
                
                <Mensagem iniciar={gerarGrafico1} />
                <Mensagem2 iniciar={gerarGrafico2} />

            </div>
            <ConfigAntena isAntena={isAntena} isOpen={openConfigAntena} setConfigAntena={() => setOpenConfigAntena(!openConfigAntena)}
            setGerarGrafico1={setGerarGrafico1} setGerarGrafico2={setGerarGrafico2} setIniciarCronometro={setIniciarCronometro} setPararCronometro={setPararCronometro}/>

            <Save isAntena={isAntena} isOpen={openSave} setSave={() => setOpenSave(!openSave)}
            setSalvo={() => setOpenSalvo(true)}/>
            <Salvo isOpen={openSalvo} setSave={() => setOpenSalvo(!openSalvo)} />
        
        </div>
    )
};