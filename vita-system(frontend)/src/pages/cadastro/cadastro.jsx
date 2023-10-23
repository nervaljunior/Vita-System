import './cadastro.css'
import React, { useState } from "react";
import axios from "axios";
import { useNavigate } from 'react-router-dom';

export const Cadastro = () => {

    const navigate = useNavigate();
    const [username, setUsername] = useState("");
    const [id, setId] = useState("");
    const [password, setPassword] = useState("");
    const [confirmPassword, setConfirmPassword] = useState("");
    const [isAdmin, setIsAdmin] = useState(false);
    const [isOperator, setIsOperator] = useState(false);
    const [cadError, setCadError] = useState(false);

    const handleCadastro = async () => {
      try {
        // Verificar se as senhas coincidem
        if (password !== confirmPassword) {
          console.log("As senhas não coincidem");
          return;
        }
  
        // Fazer a solicitação de cadastro para o servidor backend
        const response = await axios.post("http://localhost:5000/register", {
          username,
          id,
          password,
          role: isAdmin ? 1 : 0,
        });
  
        // Verificar a resposta do servidor
        if (response.data.message === "User registered successfully") {
          // Cadastro bem-sucedido
          console.log("Usuário cadastrado com sucesso");
          // Redirecionar para a página de adm
          return navigate('/adm')
        } else {
          // Tratar mensagem de erro
          console.log("Erro ao cadastrar usuário");
          setCadError(true);
        }
      } catch (error) {
        // Tratar erros de solicitação
        console.error("Erro ao fazer o cadastro:", error);
        setCadError(true);
      }
    };
  
    return (
      <div id="pagecad" className="box">
        <h1 id="h1cadastro">CADASTRAR</h1>

        <input
          type="text"
          id="usercad"
          placeholder="NOME COMPLETO"
          value={username}
          onChange={(e) => setUsername(e.target.value)}
        />
        <input
          type="number"
          id="matriculacad"
          placeholder="MATRÍCULA"
          value={id}
          onChange={(e) => setId(e.target.value)}
        />
        <input
          type="password"
          id="passwordcad"
          placeholder="SENHA (MÁXIMO 10 CARACTERES)"
          value={password}
          onChange={(e) => setPassword(e.target.value)}
        />
        <input
          type="password"
          id="confirmpasswordcad"
          placeholder="CONFIRMAR SENHA"
          value={confirmPassword}
          onChange={(e) => setConfirmPassword(e.target.value)}
        />
  
  <input
        type="checkbox"
        id="adminput"
        checked={isAdmin}
        onChange={(e) => setIsAdmin(e.target.checked)}
      />
      <label
        htmlFor="adminput"
        id="admlabel"
        className={isAdmin ? "checked" : ""}
      >
        ADMINISTRADOR
      </label>

      <input
        type="checkbox"
        id="funcinput"
        checked={isOperator}
        onChange={(e) => setIsOperator(e.target.checked)}
      />
      <label
        htmlFor="funcinput"
        id="funclabel"
        className={isOperator ? "checked" : ""}
      >
        OPERADOR
      </label>
      
      {cadError && <p id='pcadastro'>Erro ao fazer cadastro</p>}

        <button type="submit" id="buttoncad" onClick={handleCadastro}>
          ADICIONAR
        </button>

      </div>
    );
};