import React, { useState, useContext } from "react";
import { AuthContext } from "../../src/context/AuthContext";

const Login = () => {
  const [email, setEmail] = useState("");
  const [password, setPassword] = useState('');

  const { handleLogin } = useContext(AuthContext);

  async function signIn(event) {
    event.preventDefault();
    try {
      handleLogin(email, password);
      
    } catch (err) {
      console.log(err);
    }
  }

  return (
    <>
      <div>
        <h1>Login</h1>
        <form onSubmit={signIn}>
        <input
          type="text"
          placeholder="Email"
          value={email}
          onChange={(e) => setEmail(e.target.value)}
        />
        <input
          type="password"
          placeholder="Senha"
          value={password}
          onChange={(e) => setPassword(e.target.value)}
        />
        <button type='submit'>Login</button>
        </form>
      </div>
    </>
  );
};

export default Login;
