import '../styles/globals.css'
import { AuthContextProvider } from '../src/context/AuthContext';
import { AuthProvider } from '../src/context/AuthContext';
function MyApp({ Component, pageProps }) {
  return (
    <AuthProvider>
      <Component {...pageProps} />
    </AuthProvider>
  );
}

export default MyApp
