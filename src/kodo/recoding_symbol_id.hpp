// Copyright Steinwurf ApS 2011-2012.
// Distributed under the "STEINWURF RESEARCH LICENSE 1.0".
// See accompanying file LICENSE.rst or
// http://www.steinwurf.com/licensing

#ifndef KODO_RECODING_SYMBOL_ID_HPP
#define KODO_RECODING_SYMBOL_ID_HPP

#include <cstdint>

#include <boost/random/mersenne_twister.hpp>
#include <boost/random/uniform_int_distribution.hpp>

#include <fifi/fifi_utils.hpp>

namespace kodo
{

    /// @brief Randomly recombines existing coding coefficients to
    ///        allow a decoder to produce recoded packets.
    ///
    /// @ingroup symbol_id_layers
    /// @ingroup factory_layers
    template<class SuperCoder>
    class recoding_symbol_id : public SuperCoder
    {
    public:

        /// The value type
        typedef typename SuperCoder::value_type value_type;

        /// The finite field type
        typedef typename SuperCoder::field_type field_type;

        /// Pointer to coder produced by the factories
        typedef typename SuperCoder::pointer pointer;

    public:

        /// @ingroup factory_layers
        /// The factory layer associated with this coder.
        class factory : public SuperCoder::factory
        {
        public:

            /// @copydoc layer::factory::factory()
            factory(uint32_t max_symbols, uint32_t max_symbol_size)
                : SuperCoder::factory(max_symbols, max_symbol_size)
                { }

            /// @copydoc layer::factory::max_id_size()
            uint32_t max_id_size() const
                {
                    return SuperCoder::max_coefficients_size();
                }

            /// @copydoc layer::factory::build(uint32_t,uint32_t)
            pointer build(uint32_t symbols, uint32_t symbol_size)
                {
                    pointer coder =
                        SuperCoder::factory::build(symbols, symbol_size);

                    coder->Initialize_storage(
                        SuperCoder::factory::max_coefficients_size());

                    return coder;
                }
        };

    public:

        /// @copydoc layer::initialize()
        void initialize(uint32_t symbols, uint32_t symbol_size)
            {
                SuperCoder::initialize(symbols, symbol_size);

                m_id_size = SuperCoder::coefficients_size();
                assert(m_id_size > 0);
            }

        /// Will write the recoded encoding vector (symbol id) into the
        /// symbol_id buffer. The coding coefficients used to produce the
        /// recoded encoding vector will be available in the
        /// symbol_coefficient buffer.
        ///
        /// @copydoc layer::write_id(uint8_t*, uint8_t*)
        uint32_t write_id(uint8_t *symbol_id, uint8_t **symbol_coefficients)
            {
                assert(symbol_id != 0);
                assert(symbol_coefficients != 0);

                // Zero the symbol id
                std::fill_n(symbol_id, m_id_size, 0);

                // Check the number of symbols stored
                uint32_t symbol_count = SuperCoder::symbol_count();

                if(symbol_count == 0)
                {
                    // Nothing we can do - we just return the zero'ed
                    // symbol coefficients and id
                    *symbol_coefficients = symbol_id;
                    return m_id_size;
                }
                else if(symbol_count < SuperCoder::symbols())
                {
                    SuperCoder::generate_partial(&m_coefficients[0]);
                }
                else
                {
                    SuperCoder::generate(&m_coefficients[0]);
                }

                // Create the recoded symbol id
                value_type *recode_id
                    = reinterpret_cast<value_type*>(symbol_id);

                for(uint32_t i = 0; i < SuperCoder::symbols(); ++i)
                {
                    value_type c =
                        fifi::get_value<field_type>(recode_id, i);

                    if(!c)
                    {
                        continue;
                    }

                    assert(SuperCoder::symbol_exists(i));

                    value_type *source_id =
                        SuperCoder::coefficients_value( i );

                    if(fifi::is_binary<field_type>::value)
                    {
                        SuperCoder::add(
                            recode_id, source_id,
                            SuperCoder::coefficients_length());
                    }
                    else
                    {
                        SuperCoder::multiply_add(
                            recode_id, source_id, c,
                            SuperCoder::coefficients_length());

                    }
                }


                *symbol_coefficients = &m_coefficients[0];
                return m_id_size;
            }


        /// @copydoc layer::id_size()
        uint32_t id_size() const
            {
                return m_id_size;
            }

    private:

        /// Initialize the coefficient storage
        void Initialize_storage(uint32_t max_coefficients_size)
            {
                assert(max_coefficients_size > 0);

                // Note, that resize will not re-allocate anything
                // as long as the sizes are not larger than
                // previously. So this call should only have an
                // effect the first time this function is called.
                m_coefficients.resize(max_coefficients_size);
            }

    protected:

        /// The number of bytes needed to store the symbol id
        /// coding coefficients
        uint32_t m_id_size;

        /// Buffer for the recoding coefficients
        std::vector<uint8_t> m_coefficients;
    };

}

#endif

