import React from "react";
import { useState } from "react";
import { FcCheckmark } from 'react-icons/fc'
import { AiOutlineClose } from 'react-icons/ai'
import './save.css'
import axios from "axios";

export const Save = ({ isOpen, setSave, isAntena, setSalvo}) => {


    const naoSalvar = () => {
        if(isAntena === "antena1") {
            axios.post("http://localhost:4000/configantena/save1", {
                save: false
            })
        }
        else if (isAntena === "antena2") {
            axios.post("http://localhost:4000/configantena/save2", {
                save: false
            })
        }
        setSave(false);
    };

    const salvar = () => {
        if(isAntena === "antena1") {
            axios.post("http://localhost:4000/configantena/save1", {
                save: true
            })
        }
        else if (isAntena === "antena2") {
            axios.post("http://localhost:4000/configantena/save2", {
                save: true
            })
        }
        setSave(false);
        setSalvo();
    };


        
    if(isOpen)   
        return (
            <div id="background">
                <div className="boxSave">
                    <h1 id='tituloSave'>Gerar Relatório?</h1>
                    <button type="submit" className="botaoSim" onClick={salvar}>
                    <span><FcCheckmark />Sim</span></button>
                    
                    <button type="submit" class='botaoNao' onClick={naoSalvar}>
                    <span><AiOutlineClose />Nao</span></button>
                </div>
            </div>
        )
        
    return null
};