module.exports = {
  content: ['./index.html', './src/**/*.{js,ts,jsx,tsx,css,md,mdx,html,json,scss}'],
  darkMode: 'class',
  theme: {
    extend: {
      colors: {
        transparent: 'transparent',
        current: 'currentColor',
        white: '#ffffff',
        /* primary: {
          DEFAULT: '#31efb8',
          50: '#ededed',
          100: '#dfdfdf',
          200: '#c3c3c4',
          300: '#a6a6a8',
          400: '#8a8a8c',
          500: '#6e6e70',
          600: '#525254',
          700: '#363637',
          800: '#1e2023',
          900: '#0e0e10',
        }, */
        primary: {
          DEFAULT: '#31efb8',
          50: '#f3f3f4',
          100: '#dbdcdd',
          200: '#b5b6b9',
          300: '#8d9094',
          400: '#575c62',
          500: '#30363d',
          600: '#21262d',
          700: '#171b22',
          800: '#0e1117',
          900: '#010409'
        }
      },
      gridTemplateColumns: {
        'menu-item': '1.5rem 1fr 1.5rem'
      }
    }
  },
  plugins: []
};
