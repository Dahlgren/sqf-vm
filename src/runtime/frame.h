#pragma once

#include "instruction_set.h"
#include "value_scope.h"

namespace sqf::runtime
{
	class context;
	class frame : public sqf::runtime::value_scope
	{
	public:
		class behavior
		{
		public:
			enum class result
			{
				/// <summary>
				/// Tells that nothing should happen on Frame level.
				/// </summary>
				ok,
				/// <summary>
				/// Tells that Frame should seek to start.
				/// </summary>
				seek_start,
				/// <summary>
				/// Tells that Frame should seek to end.
				/// </summary>
				seek_end,
				/// <summary>
				/// Tells that the active instruction_set should be changed to the one
				/// provided by this behavior.
				/// </summary>
				exchange
			};
			virtual sqf::runtime::instruction_set get_instruction_set() = 0;
			virtual result enact(sqf::runtime::context& context, const sqf::runtime::frame& frame) = 0;
			behavior() = default;
		};
		enum class result
		{
			/// <summary>
			/// Something moved wrong
			/// </summary>
			error = -1,
			/// <summary>
			/// Tells that the current instruction-set is done.
			/// Will always be returned once last instruction was reached.
			/// </summary>
			/// <remarks>
			/// If frame contains 3 instructions, the 3th call may return done.
			/// </remarks>
			done,
			/// <summary>
			/// Returned on success.
			/// </summary>
			ok
		};
	private:
		sqf::runtime::instruction_set m_instruction_set;
		sqf::runtime::instruction_set::reverse_iterator m_iterator;
		std::shared_ptr<behavior> m_enter_behavior;
		std::shared_ptr<behavior> m_exit_behavior;
		std::shared_ptr<behavior> m_error_behavior;
	public:
		frame() : frame({}, {}, {}, {}) {}
		frame(std::shared_ptr<behavior> enter_behavior, sqf::runtime::instruction_set instruction_set) : frame(enter_behavior, instruction_set, {}, {}) {}
		frame(sqf::runtime::instruction_set instruction_set, std::shared_ptr<behavior> exit_behavior) : frame({}, instruction_set, exit_behavior, {}) {}
		frame(sqf::runtime::instruction_set instruction_set) : frame({}, instruction_set, {}, {}) {}
		frame(std::shared_ptr<behavior> enter_behavior,
			  sqf::runtime::instruction_set instruction_set,
			  std::shared_ptr<behavior> exit_behavior,
			  std::shared_ptr<behavior> error_behavior)
			:
			m_instruction_set(instruction_set),
			m_iterator(instruction_set.rbegin()),
			m_enter_behavior(enter_behavior),
			m_exit_behavior(exit_behavior),
			m_error_behavior(error_behavior)
		{}

		bool can_recover_runtime_error() { return m_error_behavior != nullptr; }
		result recover_runtime_error(context& context)
		{
			if (!m_error_behavior) { return result::error; }
			switch (m_error_behavior->enact(context, *this))
			{
			case behavior::result::seek_end:
				m_iterator = m_instruction_set.rend();
				return result::done;
			case behavior::result::seek_start:
				m_iterator = m_instruction_set.rbegin();
				return result::ok;
			case behavior::result::exchange:
				m_instruction_set = m_enter_behavior->get_instruction_set();
				m_iterator = m_instruction_set.rbegin();
				return result::ok;
			}
			return result::ok;
		}

		sqf::runtime::instruction_set::reverse_iterator peek() const { bool flag; return peek(flag); }
		sqf::runtime::instruction_set::reverse_iterator peek(bool& success) const
		{
			auto it = m_iterator == m_instruction_set.rend() ? m_iterator : m_iterator + 1;
			success = it != m_instruction_set.rend();
			return it;
		}


		sqf::runtime::instruction_set::reverse_iterator current() const { return m_iterator; }

		/// <summary>
		/// Moves current to next instruction.
		/// Will not trigger possible attached behaviors
		/// </summary>
		/// <returns>Enum value describing the success state of the operation.</returns>
		result next()
		{
			if (m_iterator == m_instruction_set.rend()) { return result::done; }
			return ++m_iterator == m_instruction_set.rend() ? result::done : result::ok;
		}
		/// <summary>
		/// Moves current to next instruction.
		/// </summary>
		/// <returns>Enum value describing the success state of the operation.</returns>
		result next(context& context)
		{
			if (m_iterator == m_instruction_set.rbegin())
			{
				switch (m_enter_behavior->enact(context, *this))
				{
				case behavior::result::seek_end:
					m_iterator = m_instruction_set.rend();
					break;
				case behavior::result::seek_start:
					m_iterator = m_instruction_set.rbegin();
					break;
				case behavior::result::exchange:
					m_instruction_set = m_enter_behavior->get_instruction_set();
					m_iterator = m_instruction_set.rbegin();
					break;
				}
			}
			if (m_iterator != m_instruction_set.rend())
			{
				auto res = next();
				if (m_iterator == m_instruction_set.rend())
				{
					switch (m_enter_behavior->enact(context, *this))
					{
					case behavior::result::seek_end:
						m_iterator = m_instruction_set.rend();
						return result::done;
					case behavior::result::seek_start:
						m_iterator = m_instruction_set.rbegin();
						return next();
					case behavior::result::exchange:
						m_instruction_set = m_enter_behavior->get_instruction_set();
						m_iterator = m_instruction_set.rbegin();
						return next();
					}
				}
				return res;
			}
			else
			{
				return next();
			}
		}
		/// <summary>
		/// Reversed sqf::runtime::frame::next().
		/// Moves current to previous instruction.
		/// Will not trigger possible attached behaviors
		/// </summary>
		/// <returns>Enum value describing the success state of the operation.</returns>
		result previous()
		{
			if (m_iterator == m_instruction_set.rbegin()) { return result::done; }
			return --m_iterator == m_instruction_set.rbegin() ? result::done : result::ok;
		}
	};
}