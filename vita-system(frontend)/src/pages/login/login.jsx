import React, { useState } from 'react';
import { useNavigate } from 'react-router-dom';
import { useAuth } from '../../Auth/context'
import './login.css'



export const Login = () => { 

  const login = useAuth();
  const navigate = useNavigate();
  const [username, setUsername] = useState('');
  const [password, setPassword] = useState('');
  const [loginSuccess, setLoginSuccess] = useState(false);
  const [loginError, setLoginError] = useState(false);

  const handleLogin = async () => {
    try {
      // Fazer a solicitação de login para o servidor backend
      const response = await login.login({ username, password });

      if (response.data.message === 'Login successful') {
        // Login bem-sucedido
        setLoginSuccess(true);
        if (response.data.role === 'Administrator') {
          // Redirecionar para a página de administrador
          return navigate('/adm');
        } else if (response.data.role === 'User') {
          return navigate('/func');
        }
      } else {
        // Login falhou
        setLoginSuccess(false);
        setLoginError(true);
      }
      } catch (error) {
        // Tratar erros de solicitação
          console.error('Erro ao fazer login:', error);
          setLoginSuccess(false);
          setLoginError(true);
      }
  };
    return (   
       <div>
            <div id="login" className="box">
                <h1>Faça seu Login</h1>
                {loginError && <p id='invalid'>Usuário ou senha inválidos</p>}
                <input 
                    type='text'
                    id="usernamelogin"
                    placeHolder="USERNAME"
                    value={username}
                    onChange={(e) => setUsername(e.target.value)} ></input>

                <input 
                    type='password'
                    id="senhalogin" 
                    placeHolder="SENHA" 
                    value={password}
                    onChange={(e) => setPassword(e.target.value)}
                    ></input>
                <button type='submit' id='buttonlogin' onClick={handleLogin} >
                    LOGIN
                </button>
            </div>
        </div>
    )
    }
