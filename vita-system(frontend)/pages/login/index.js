import React, { useState, useContext } from "react";
import { AuthContext } from "../../src/context/AuthContext";
import styles from './Login.module.css';
import Topbar from "../../src/components/Topbar/Topbar";

const Login = () => {
  
  const [loginError, setLoginError] = useState('');
  const [email, setEmail] = useState("");
  const [password, setPassword] = useState('');

  const { handleLogin } = useContext(AuthContext);

  async function signIn(event) {
    event.preventDefault();
    try {
      handleLogin(email, password);
      
    } catch (err) {
      setLoginError('Erro ao fazer login. Verifique suas credenciais.');
      console.log(err);
    }

    
  }

  return (

    <>
    <Topbar /> 
    <div className={styles.body}>
      <div className={styles.container}>
        <h1 className={styles.h1}>Login</h1>
        <form className={styles.form} onSubmit={signIn}>
          <label className={styles.label}>Email</label>
          <input
            type="text"
            value={email}
            onChange={(e) => setEmail(e.target.value)}
            className={styles.input}
          />
          <label className={styles.label}>Senha</label>
          <input 
            type="password"
            value={password}
            onChange={(e) => setPassword(e.target.value)}
            className={styles.input}
          />
          <button className={`${styles.loginButton} loginButton`} type='submit'>Login</button>
          {loginError && <p className={styles.errorMessage}>{loginError}</p>}
        </form>
        <p>
          Não tem uma conta? <a href="/registro" className={styles.signupLink}>Cadastre-se</a>
        </p>
      </div>
    </div>
    </>
  );
};

export default Login;


